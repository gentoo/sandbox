/*
 * open*() pre-check.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

bool sb_openat_pre_check(const char *func, const char *pathname, int dirfd, int flags)
{
	/* If we're not trying to create, fail normally if
	 * file does not stat
	 */
	if (flags & O_CREAT)
		return true;

	save_errno();

	/* Expand the dirfd path first */
	char dirfd_path[SB_PATH_MAX];
	switch (resolve_dirfd_path(dirfd, pathname, dirfd_path, sizeof(dirfd_path))) {
		case -1:
			sb_debug_dyn("EARLY FAIL: %s(%s) @ resolve_dirfd_path: %s\n",
				func, pathname, strerror(errno));
			return false;
		case 0:
			pathname = dirfd_path;
			break;
	}

	/* Doesn't exist -> skip permission checks */
	struct stat st;
	if (((flags & O_NOFOLLOW) ? lstat(pathname, &st) : stat(pathname, &st)) == -1) {
		sb_debug_dyn("EARLY FAIL: %s(%s): %s\n",
			func, pathname, strerror(errno));
		return false;
	}

	restore_errno();

	return true;
}
