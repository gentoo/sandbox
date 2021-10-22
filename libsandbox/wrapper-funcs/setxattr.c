/*
 * setxattr() wrapper.
 *
 * Copyright 1999-2021 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *path, const char *name, const void *value, size_t size, int flags
#define WRAPPER_ARGS path, name, value, size, flags
#define WRAPPER_SAFE() SB_SAFE(path)
#include "__wrapper_simple.c"
