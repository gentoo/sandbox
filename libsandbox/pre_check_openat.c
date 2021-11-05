/*
 * open*() pre-check.
 *
 * Copyright 1999-2012 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"
#include "libsandbox.h"
#include "wrappers.h"

bool sb_openat_pre_check(const char *func, const char *pathname, int dirfd, int flags)
{
	/* If we're not trying to create, fail normally if file does not stat */
	if (flags & O_CREAT)
		return true;

	save_errno();

	/* Doesn't exist -> skip permission checks */
	if (faccessat(dirfd, pathname, F_OK, (flags & O_NOFOLLOW) ? AT_SYMLINK_NOFOLLOW : 0) == -1) {
		sb_debug_dyn("EARLY FAIL: %s(%s): %s\n", func, pathname, strerror(errno));
		return false;
	}

	restore_errno();

	return true;
}
