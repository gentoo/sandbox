/*
 * linkat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags
#define WRAPPER_ARGS olddirfd, oldpath, newdirfd, newpath, flags
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_AT(newdirfd, newpath, flags)
#include "__wrapper_simple.c"
