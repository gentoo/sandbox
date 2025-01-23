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
	save_errno();

	if (sb_exists(dirfd, pathname, AT_SYMLINK_NOFOLLOW) >= 0) {
		errno = EEXIST;
		return false;
	}

	restore_errno();

	return true;
}
