/*
 * remove() wrapper.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *pathname
#define WRAPPER_ARGS pathname
#define WRAPPER_SAFE() SB_SAFE(pathname)
#define WRAPPER_PRE_CHECKS() sb_unlinkat_pre_check(STRING_NAME, pathname, AT_FDCWD)
#include "__wrapper_simple.c"
