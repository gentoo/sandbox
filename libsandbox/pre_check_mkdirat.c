/*
 * mkdir*() pre-check.
 *
 * Copyright 1999-2012 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"
#include "libsandbox.h"
#include "wrappers.h"

bool sb_mkdirat_pre_check(const char *func, const char *pathname, int dirfd)
{
	char canonic[SB_PATH_MAX];

	save_errno();

	/* Check incoming args against common *at issues */
	char dirfd_path[SB_PATH_MAX];
	if (!sb_common_at_pre_check(func, &pathname, dirfd, dirfd_path, sizeof(dirfd_path)))
		return false;

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

		/* Hmm, is this a broken symlink we're trying to extend ?
		 * Or is this a path like "foo/.." ?
		 */
		if (stat(pathname, &st) != 0) {
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
