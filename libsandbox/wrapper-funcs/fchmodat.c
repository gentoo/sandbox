/*
 * fchmodat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int dirfd, const char *path, mode_t mode, int flags
#define WRAPPER_ARGS dirfd, path, mode, flags
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_AT(dirfd, path, flags)
#include "__wrapper_simple.c"
