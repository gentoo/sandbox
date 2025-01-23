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
	save_errno();

	char canonic[SB_PATH_MAX];
	if (sb_realpathat(dirfd, pathname, canonic, sizeof(canonic), AT_SYMLINK_NOFOLLOW, false)) {
		if (!strcmp(canonic, "/dev/null") || !strcmp(canonic, "/dev/zero")) {
			errno = EACCES;
			sb_debug_dyn("EARLY FAIL: %s(%s): %s\n",
				func, canonic, strerror(errno));
			return false;
		}
	}

	restore_errno();

	return true;
}
