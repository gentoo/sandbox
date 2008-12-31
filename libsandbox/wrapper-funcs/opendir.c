/*
 * opendir() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *name
#define WRAPPER_ARGS name
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE(name)
#define WRAPPER_RET_TYPE DIR *
#define WRAPPER_RET_DEFAULT NULL
#include "__wrapper_simple.c"
