/* These functions find the absolute path to the current working directory.  */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

#include "config.h"
#include <sys/param.h>
#include "localdecls.h"

/* Modified: 08 June 2005; Martin Schlemmer <azarah@gentoo.org>
 * 
 *  Cleaned up unneeded stuff.	Add a wrapper to try and detect when
 *  we have a kernel whose getcwd system call do not handle directory
 *  names longer than PATH_MAX, and if so, use our generic version.
 */

#ifndef __set_errno
# define __set_errno(val) errno = (val)
#endif

/* If the syscall is not present, we have to walk up the
 * directory tree till we hit the root.  Now we _could_
 * use /proc/self/cwd if /proc is mounted... That approach
 * is left an an exercise for the reader... */


/* Seems a few broken filesystems (like coda) don't like this */
/*#undef FAST_DIR_SEARCH_POSSIBLE on Linux */


/* Routine to find the step back down */
SB_STATIC char *search_dir(dev_t this_dev, ino_t this_ino, char *path_buf, size_t path_size)
{
	DIR *dp;
	struct dirent *d;
	char *ptr;
	size_t slen;
	struct stat st;

#ifdef FAST_DIR_SEARCH_POSSIBLE
	/* The test is for ELKS lib 0.0.9, this should be fixed in the real kernel */
	size_t slow_search = (sizeof(ino_t) != sizeof(d->d_ino));
#endif

//	if (stat(path_buf, &st) < 0) {
//		goto oops;
//	}
#ifdef FAST_DIR_SEARCH_POSSIBLE
	if (this_dev != st.st_dev)
		slow_search = 1;
#endif

	slen = strlen(path_buf);
	ptr = path_buf + slen - 1;
	if (*ptr != '/') {
		if (slen + 2 > path_size) {
			goto oops;
		}
		snprintf(++ptr, 2, "/");
		slen++;
	}
	slen++;

#ifdef OUTSIDE_LIBSANDBOX
	dp = opendir(path_buf);
#else
	check_dlsym(opendir);
	dp = true_opendir(path_buf);
#endif
	if (dp == 0) {
	    goto oops;
	}

	while ((d = readdir(dp)) != 0) {
#ifdef FAST_DIR_SEARCH_POSSIBLE
		if (slow_search || this_ino == d->d_ino) {
#endif
			if (slen + strlen(d->d_name) > path_size) {
			    goto oops;
			}
			snprintf(ptr + 1, sizeof(d->d_name) + 1, "%s", d->d_name);
			if (lstat(path_buf, &st) < 0)
				continue;
			if (st.st_ino == this_ino && st.st_dev == this_dev) {
				closedir(dp);
				return path_buf;
			}
#ifdef FAST_DIR_SEARCH_POSSIBLE
		}
#endif
	}

	closedir(dp);
	return 0;

oops:
	__set_errno(ERANGE);
	return 0;
}

/* Routine to go up tree */
SB_STATIC char *recurser(char *path_buf, size_t path_size, dev_t root_dev, ino_t root_ino)
{
	struct stat st;
	dev_t this_dev;
	ino_t this_ino;

	if (lstat(path_buf, &st) < 0) {
	    if (errno != EFAULT)
		goto oops;
	    return 0;
	}
	this_dev = st.st_dev;
	this_ino = st.st_ino;
	if (this_dev == root_dev && this_ino == root_ino) {
		if (path_size < 2) {
		    goto oops;
		}
		snprintf(path_buf, 2, "/");
		return path_buf;
	}
	if (strlen(path_buf) + 4 > path_size) {
	    goto oops;
	}
	snprintf(path_buf, 4, "/..");
	if (recurser(path_buf, path_size, root_dev, root_ino) == 0)
		return 0;

	return search_dir(this_dev, this_ino, path_buf, path_size);
oops:
	__set_errno(ERANGE);
	return 0;
}

SB_STATIC inline
size_t __syscall_egetcwd(char * buf, unsigned long size)
{
    size_t len;
    char *cwd;
    struct stat st;
    size_t olderrno;

    olderrno = errno;
    len = -1;
    cwd = recurser(buf, size, st.st_dev, st.st_ino);
    if (cwd) {
	len = strlen(buf);
	__set_errno(olderrno);
    }
    return len;
}

SB_STATIC char *__egetcwd(char *buf, size_t size)
{
    size_t ret;
    char *path;
    size_t alloc_size = size;

    if (size == 0) {
	if (buf != NULL) {
	    __set_errno(EINVAL);
	    return NULL;
	}
	alloc_size = SB_PATH_MAX;
    }
    path=buf;
    if (buf == NULL) {
	path = malloc(alloc_size);
	if (path == NULL)
	    return NULL;
    }
    ret = __syscall_egetcwd(path, alloc_size);
    if (ret >= 0)
    {
	if (buf == NULL && size == 0)
	    buf = realloc(path, ret);
	if (buf == NULL)
	    buf = path;
	return buf;
    }
    if (buf == NULL)
	free (path);
    return NULL;
}

SB_STATIC char *egetcwd(char *buf, size_t size)
{
	struct stat st;
	char *tmpbuf;

	__set_errno(0);
	tmpbuf = getcwd(buf, size);

	if (tmpbuf) {
		lstat(buf, &st);
	} else {
		return tmpbuf;
	}

	if (errno) {
		/* If lstat() failed with eerror = ENOENT, then its
		 * possible that we are running on an older kernel,
		 * so use our generic version which *should* not fail.
		 */
		if (errno == ENOENT) {
			return __egetcwd(buf, size);
		} else {
			return tmpbuf;
		}
	}

	return tmpbuf;
}

