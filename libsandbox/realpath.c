#include <headers.h>
#include "libsandbox.h"
#include "wrappers.h"

static ssize_t zreadlink(const char *pathname, char *buf, size_t bufsiz)
{
	ssize_t s = readlink(pathname, buf, bufsiz);
	if (s == bufsiz) {
		errno = ENAMETOOLONG;
		return -1;
	}
	if (s >= 0)
		buf[s] = '\0';
	return s;
}

static ssize_t zreadlinkat(int dirfd, const char *pathname, char *buf, size_t bufsiz)
{
	ssize_t s = readlinkat(dirfd, pathname, buf, bufsiz);
	if (s == bufsiz) {
		errno = ENAMETOOLONG;
		return -1;
	}
	if (s >= 0)
		buf[s] = '\0';
	return s;
}

static ssize_t get_fd_path(int fd, char *buf, size_t bufsiz)
{
	if (fd == AT_FDCWD)
		return zreadlink("/proc/self/cwd", buf, bufsiz);

	if (fd < 0) {
		errno = EBADF;
		return -1;
	}

	char fdpath[26];
	sprintf(fdpath, "/proc/self/fd/%i", fd);

	ssize_t s = zreadlink(fdpath, buf, bufsiz);
	if (s < 0 && errno == ENOENT)
		errno = EBADF;

	return s;
}

bool sb_abspathat(int dirfd, const char *restrict path, char *buf, size_t bufsiz)
{
	size_t pathlen = path ? strlen(path) : 0;

	if (pathlen >= bufsiz) {
		errno = ENAMETOOLONG;
		return false;
	}

	if (path && path[0] == '/') {
		memcpy(buf, path, pathlen + 1);
		return true;
	}

	ssize_t s = get_fd_path(dirfd, buf, bufsiz);
	if (s < 0)
		return false;

	if (buf[0] != '/' || !path)
		return true;

	if (s > 1)
		buf[s++] = '/';

	if (s + pathlen >= bufsiz) {
		errno = ENAMETOOLONG;
		return false;
	}

	memcpy(buf + s, path, pathlen + 1);
	return true;
}

static bool chase_linkfd(int linkfd, char *buf, size_t bufsiz)
{
	ssize_t llen = get_fd_path(linkfd, buf, bufsiz);

	if (llen < 0)
		return false;

	if (buf[0] != '/')
		return true;

	char target[PATH_MAX];
	ssize_t tlen = zreadlinkat(linkfd, "", target, sizeof(target));

	int i;

	for (i = 40; i > 0 && tlen >= 0; --i) {
		char *dst = (target[0] == '/' ? buf : strrchr(buf, '/') + 1);

		if (dst < buf)
			dst = buf;

		if (tlen >= bufsiz - (dst - buf)) {
			errno = ENAMETOOLONG;
			return false;
		}

		memcpy(dst, target, tlen + 1);

		tlen = zreadlink(buf, target, sizeof(target));
	}

	if (i <= 0) {
		errno = ELOOP;
		return false;
	}

	return true;
}

static int get_path_fd(int dirfd, const char *restrict path, int atflags, int oflags)
{
	if (path == NULL || (path[0] == '\0' && (atflags & AT_EMPTY_PATH)))
		return dirfd;

	oflags |= O_PATH|O_CLOEXEC;
	if (atflags & AT_SYMLINK_NOFOLLOW)
		oflags |= O_NOFOLLOW;

	return sb_unwrapped_openat(dirfd, path, oflags, 0);
}

bool sb_realpathat(int dirfd, const char *restrict path, char *buf, size_t bufsiz, int flags, bool create)
{
	const char *bname = NULL;

	int pathfd = get_path_fd(dirfd, path, flags, 0);
	if (pathfd != AT_FDCWD && pathfd < 0 && errno == ENOENT) {
		if (!(flags & AT_SYMLINK_NOFOLLOW)) {
			pathfd = get_path_fd(dirfd, path, flags, O_NOFOLLOW);
			if (pathfd == AT_FDCWD || pathfd >= 0) {
				if (!chase_linkfd(pathfd, buf, bufsiz))
					return false;
				path = buf;
			}
		}

		if (create) {
			char *slash = strrchr(path, '/');
			if (slash) {
				bname = slash + 1;

				const char *dname;

				if (slash > path) {
					ptrdiff_t len = slash - path;
					if (len >= bufsiz) {
						errno = ENAMETOOLONG;
						return false;
					}
					memcpy(buf, path, len);
					buf[len] = '\0';
					dname = buf;
				}
				else
					dname = "/";

				pathfd = get_path_fd(dirfd, dname, 0, O_DIRECTORY);
			}
			else {
				bname = path;
				pathfd = dirfd;
			}
		}
	}

	if (pathfd != AT_FDCWD && pathfd < 0)
		return false;

	bool ret = true;

	ssize_t len = get_fd_path(pathfd, buf, bufsiz);
	if (len < 0)
		ret = false;
	else if (buf[0] == '/' && bname && bname[0]) {
		if (len > 1)
			buf[len++] = '/';
		size_t blen = strlen(bname);
		if (blen >= bufsiz - len) {
			errno = ENAMETOOLONG;
			ret = false;
		}
		else
			memcpy(buf + len, bname, blen + 1);
	}

	if (pathfd != dirfd) {
		int save_errno = errno;
		close(pathfd);
		errno = save_errno;
	}

	return ret;
}
