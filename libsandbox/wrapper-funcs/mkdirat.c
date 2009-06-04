/*
 * mkdirat() wrapper.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO /* let mkdir() use us */
# define WRAPPER_ARGS_PROTO int dirfd, const char *pathname, mode_t mode
# define WRAPPER_ARGS dirfd, pathname, mode
# define WRAPPER_SAFE() SB_SAFE_AT(dirfd, pathname, 0)
#else
# define dirfd AT_FDCWD
#endif

#define WRAPPER_PRE_CHECKS() sb_mkdirat_pre_check(STRING_NAME, pathname, dirfd)

#include "__wrapper_simple.c"

#undef dirfd
