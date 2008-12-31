/*
 * rename() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *oldpath, const char *newpath
#define WRAPPER_ARGS oldpath, newpath
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE(oldpath) && FUNCTION_SANDBOX_SAFE(newpath)
#include "__wrapper_simple.c"
