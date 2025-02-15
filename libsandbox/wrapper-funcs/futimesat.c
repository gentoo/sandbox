/*
 * futimesat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int dirfd, const char *filename, const struct timeval times[2]
#define WRAPPER_ARGS dirfd, filename, times
#define WRAPPER_SAFE() SB_SAFE_AT(dirfd, filename, 0)
#include "__wrapper_simple.c"
