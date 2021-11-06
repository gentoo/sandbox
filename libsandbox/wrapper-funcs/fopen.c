/*
 * fopen() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *pathname, const char *mode
#define WRAPPER_ARGS pathname, mode
#define WRAPPER_SAFE() SB_SAFE_OPEN_CHAR(pathname, mode)
#define WRAPPER_RET_TYPE FILE *
#define WRAPPER_RET_DEFAULT NULL
#define WRAPPER_PRE_CHECKS() sb_fopen_pre_check(STRING_NAME, pathname, mode)

#include "__wrapper_simple.c"
