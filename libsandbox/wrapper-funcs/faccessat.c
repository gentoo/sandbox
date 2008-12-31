/*
 * faccessat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int dirfd, const char *pathname, int mode, int flags
#define WRAPPER_ARGS dirfd, pathname, mode, flags
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_ACCESS_AT(dirfd, pathname, mode)
#include "__wrapper_simple.c"
