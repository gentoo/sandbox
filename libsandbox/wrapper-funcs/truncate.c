/*
 * truncate() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO
# define WRAPPER_ARGS_PROTO const char *path, off_t length
#endif
#define WRAPPER_ARGS path, length
#define WRAPPER_SAFE() SB_SAFE(path)
#include "__wrapper_simple.c"
