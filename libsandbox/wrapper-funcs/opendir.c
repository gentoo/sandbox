/*
 * opendir() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *name
#define WRAPPER_ARGS name
#define WRAPPER_SAFE() SB_SAFE(name)
#define WRAPPER_RET_TYPE DIR *
#define WRAPPER_RET_DEFAULT NULL
#define WRAPPER_PRE_CHECKS() sb_opendir_pre_check(STRING_NAME, name)

#include "__wrapper_simple.c"
