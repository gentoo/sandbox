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
	return fstatat64(dirfd, pathname, &buf, flags);
}
