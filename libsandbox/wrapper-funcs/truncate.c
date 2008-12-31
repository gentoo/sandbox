/*
 * truncate() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *path, off_t length
#define WRAPPER_ARGS path, length
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE(path)
#include "__wrapper_simple.c"
