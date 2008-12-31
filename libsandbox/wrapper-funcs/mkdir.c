/*
 * mkdir() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *pathname, mode_t mode
#define WRAPPER_ARGS pathname, mode
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE(pathname)
#define sb_mkdirat_pre_check sb_mkdir_pre_check
#include "mkdirat.c"
#undef sb_mkdirat_pre_check
