/*
 * utimes() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *filename, const struct timeval times[2]
#define WRAPPER_ARGS filename, times
#ifndef WRAPPER_SAFE
# define WRAPPER_SAFE() SB_SAFE(filename)
#endif
#include "__wrapper_simple.c"
