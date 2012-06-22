/*
 * mkdir*() pre-check.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

bool sb_mkdirat_pre_check(const char *func, const char *pathname, int dirfd)
{
	char canonic[SB_PATH_MAX];
	char dirfd_path[SB_PATH_MAX];

	save_errno();

	/* Expand the dirfd path first */
	switch (resolve_dirfd_path(dirfd, pathname, dirfd_path, sizeof(dirfd_path))) {
		case -1:
			sb_debug_dyn("EARLY FAIL: %s(%s) @ resolve_dirfd_path: %s\n",
				func, pathname, strerror(errno));
			return false;
		case 0:
			pathname = dirfd_path;
			break;
	}

	/* Then break down any relative/symlink paths */
	if (-1 == canonicalize(pathname, canonic))
		/* see comments in check_syscall() */
		if (ENAMETOOLONG != errno) {
			sb_debug_dyn("EARLY FAIL: %s(%s) @ canonicalize: %s\n",
				func, pathname, strerror(errno));
			return false;
		}

	/* XXX: Hack to prevent errors if the directory exist, and are
	 * not writable - we rather return EEXIST than fail.  This can
	 * occur if doing something like `mkdir -p /`.  We certainly do
	 * not want to pass this attempt up to the higher levels as those
	 * will trigger a sandbox violation.
	 */
	struct stat st;
	if (0 == lstat(canonic, &st)) {
		int new_errno;
		sb_debug_dyn("EARLY FAIL: %s(%s[%s]) @ lstat: %s\n",
			func, pathname, canonic, strerror(errno));

		new_errno = EEXIST;

		/* Hmm, is this a broken symlink we're trying to extend ? */
		if (S_ISLNK(st.st_mode) && stat(pathname, &st) != 0) {
			/* XXX: This awful hack should probably be turned into a
			 * common func that does a better job.  For now, we have
			 * enough crap to catch gnulib tests #297026.
			 */
			char *parent = strrchr(pathname, '/');
			if (parent && (strcmp(parent, "/.") == 0 || strcmp(parent, "/..") == 0))
				new_errno = ENOENT;
		}

		errno = new_errno;
		return false;
	}

	restore_errno();

	return true;
}
