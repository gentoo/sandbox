/*
 * _xmknod() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const int ver, const char *pathname, mode_t mode, dev_t dev
#define WRAPPER_ARGS ver, pathname, mode, dev
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE(pathname)
#include "__wrapper_simple.c"
