/*
 * getcwd() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO char *buf, size_t size
#define WRAPPER_ARGS buf, size
#define WRAPPER_RET_TYPE char *
#define WRAPPER_RET_DEFAULT NULL
#define WRAPPER_SAFE() true
/* Need to disable sandbox, as on non-linux libc's, opendir() is often
 * used by some getcwd() implementations and resolves to the sandbox
 * opendir() wrapper, causing infinite recursion and finally crashes.
 */
#define WRAPPER_SAFE_POST_EXPAND sandbox_on = false;
#define WRAPPER_POST_EXPAND      sandbox_on = true;
#include "__wrapper_simple.c"
