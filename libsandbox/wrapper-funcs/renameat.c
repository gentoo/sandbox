/*
 * renameat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int olddirfd, const char *oldpath, int newdirfd, const char *newpath
#define WRAPPER_ARGS olddirfd, oldpath, newdirfd, newpath
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_AT(olddirfd, oldpath) && FUNCTION_SANDBOX_SAFE_AT(newdirfd, newpath)
#include "__wrapper_simple.c"
