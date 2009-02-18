/*
 * open() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define _WRAPPER_ARGS_PROTO const char *pathname, int flags
#define WRAPPER_ARGS_PROTO _WRAPPER_ARGS_PROTO, ...
#define WRAPPER_ARGS_PROTO_FULL _WRAPPER_ARGS_PROTO, mode_t mode
#define WRAPPER_ARGS pathname, flags
#define WRAPPER_ARGS_FULL WRAPPER_ARGS, mode
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_OPEN_INT(pathname, flags)
#include "openat.c"
