/*
 * unlinkat() wrapper.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO /* let unlink() use us */
# define WRAPPER_ARGS_PROTO int dirfd, const char *pathname, int flags
# define WRAPPER_ARGS dirfd, pathname, flags
# define WRAPPER_SAFE() SB_SAFE_AT(dirfd, pathname, flags)
#else
# define dirfd AT_FDCWD
#endif

#define WRAPPER_PRE_CHECKS() sb_unlinkat_pre_check(STRING_NAME, pathname, dirfd)

#include "__wrapper_simple.c"

#undef dirfd
