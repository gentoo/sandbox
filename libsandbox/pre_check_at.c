/*
 * common *at() pre-checks.
 *
 * Copyright 1999-2012 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"
#include "libsandbox.h"
#include "wrappers.h"

/* We assume the parent has nested use with save/restore errno */
bool sb_common_at_pre_check(const char *func, const char **pathname, int dirfd,
                            char *dirfd_path, size_t dirfd_path_len)
{
	/* the empty path name should fail with ENOENT before any dirfd
	 * checks get a chance to run #346929
	 */
	if (*pathname && *pathname[0] == '\0') {
		errno = ENOENT;
		sb_debug_dyn("EARLY FAIL: %s(%s): %s\n",
			func, *pathname, strerror(errno));
		return false;
	}

	/* Expand the dirfd path first */
	switch (resolve_dirfd_path(dirfd, *pathname, dirfd_path, dirfd_path_len)) {
		case -1:
			sb_debug_dyn("EARLY FAIL: %s(%s) @ resolve_dirfd_path: %s\n",
				func, *pathname, strerror(errno));
			return false;
		case 0:
			*pathname = dirfd_path;
			break;
	}

	return true;
}
