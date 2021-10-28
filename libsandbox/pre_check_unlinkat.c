/*
 * unlink*() pre-check.
 *
 * Copyright 1999-2012 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"
#include "libsandbox.h"
#include "wrappers.h"

bool sb_unlinkat_pre_check(const char *func, const char *pathname, int dirfd)
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

	/* XXX: Hack to make sure sandboxed process cannot remove
	 * a device node, bug #79836. */
	if (0 == strcmp(canonic, "/dev/null") ||
	    0 == strcmp(canonic, "/dev/zero"))
	{
		errno = EACCES;
		sb_debug_dyn("EARLY FAIL: %s(%s): %s\n",
			func, pathname, strerror(errno));
		return false;
	}

	restore_errno();

	return true;
}
