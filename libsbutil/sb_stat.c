/*
 * Copyright 2023 Gentoo Authors
 * Distributed under the terms of the GNU General Public License v2
 */

#include "headers.h"
#include "sbutil.h"

/* Wrapper for faccessat to work around buggy behavior on musl */
int sb_exists(int dirfd, const char *pathname, int flags)
{
	struct stat buf;

	if (sbio_faccessat(dirfd, pathname, F_OK, flags|AT_EACCESS) == 0)
		return 0;

	/* musl's faccessat gives EINVAL when the kernel does not support
	 * faccessat2 and AT_SYMLINK_NOFOLLOW is set.
	 * https://www.openwall.com/lists/musl/2023/06/19/1 */
	if (errno != EINVAL)
		return -1;

	return fstatat(dirfd, pathname, &buf, flags);
}

int sb_fstat(int fd, mode_t *mode, int64_t *size)
{
	struct stat buf;
	if(fstat(fd, &buf))
		return -1;
	*mode = buf.st_mode;
	*size = buf.st_size;
	return 0;
}
