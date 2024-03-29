/*
 * removexattr() wrapper.
 *
 * Copyright 1999-2021 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *path, const char *name
#define WRAPPER_ARGS path, name
#define WRAPPER_SAFE() SB_SAFE(path)
#include "__wrapper_simple.c"
