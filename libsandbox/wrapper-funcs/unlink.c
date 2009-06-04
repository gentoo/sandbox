/*
 * unlink() wrapper.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *pathname
#define WRAPPER_ARGS pathname
#define WRAPPER_SAFE() SB_SAFE(pathname)
#include "unlinkat.c"
