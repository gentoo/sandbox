/*
 * Copyright 2023 Gentoo Authors
 * Distributed under the terms of the GNU General Public License v2
 */

#include "headers.h"
#include "sbutil.h"

/* Wrapper for faccessat to work around buggy behavior on musl */
int sb_exists(int dirfd, const char *pathname, int flags)
{
	struct stat64 buf;

	if (faccessat(dirfd, pathname, F_OK, flags) == 0)
		return 0;

	/* musl's faccessat gives EINVAL when the kernel does not support
	 * faccessat2 and AT_SYMLINK_NOFOLLOW is set.
	 * https://www.openwall.com/lists/musl/2023/06/19/1 */
	if (errno != EINVAL)
		return -1;

	return fstatat64(dirfd, pathname, &buf, flags);
}
