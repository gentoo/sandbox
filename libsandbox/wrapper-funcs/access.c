/*
 * access() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *pathname, int mode
#define WRAPPER_ARGS pathname, mode
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_ACCESS(pathname, mode)
#include "__wrapper_simple.c"
