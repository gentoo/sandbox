/*
 * __openat_2() wrapper (_FORTIFY_SOURCE).
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO /* let open() use us */
# define WRAPPER_ARGS_PROTO int dirfd, const char *pathname, int flags
# define WRAPPER_ARGS dirfd, pathname, flags
# define WRAPPER_SAFE() SB_SAFE_OPEN_INT_AT(dirfd, pathname, flags)
#else
# define dirfd AT_FDCWD
#endif

#include "__wrapper_simple.c"

#undef dirfd
