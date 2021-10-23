/*
 * utimensat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int dirfd, const char *filename, const struct timespec times[2], int flags
#define WRAPPER_ARGS dirfd, filename, times, flags
#ifndef WRAPPER_SAFE
# define WRAPPER_SAFE() SB_SAFE_AT(dirfd, filename, flags)
#endif
#include "__wrapper_simple.c"
