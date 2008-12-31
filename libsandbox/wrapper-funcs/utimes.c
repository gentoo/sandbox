/*
 * utimes() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *filename, const struct timeval times[]
#define WRAPPER_ARGS filename, times
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE(filename)
#include "__wrapper_simple.c"
