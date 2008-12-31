/*
 * open() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *pathname, int flags, ...
#define WRAPPER_ARGS pathname, flags
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_OPEN_INT(pathname, flags)
#include "openat.c"
