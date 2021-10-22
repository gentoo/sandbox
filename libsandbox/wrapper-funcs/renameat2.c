/*
 * renameat2() wrapper.
 *
 * Copyright 1999-2021 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int olddirfd, const char *oldpath, int newdirfd, const char *newpath, unsigned int flags
#define WRAPPER_ARGS olddirfd, oldpath, newdirfd, newpath, flags
#define WRAPPER_SAFE() (SB_SAFE_AT(olddirfd, oldpath, 0) && SB_SAFE_AT(newdirfd, newpath, 0))
#include "__wrapper_simple.c"
