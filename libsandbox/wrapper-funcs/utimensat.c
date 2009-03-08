/*
 * utimensat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int dirfd, const char *filename, const struct timespec times[], int flags
#define WRAPPER_ARGS dirfd, filename, times, flags
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_AT(dirfd, filename, flags)
#include "__wrapper_simple.c"
