/*
 * unlink() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *pathname
#define WRAPPER_ARGS pathname
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE(pathname)
#define sb_unlinkat_pre_check sb_unlink_pre_check
#include "unlinkat.c"
#undef sb_unlinkat_pre_check
