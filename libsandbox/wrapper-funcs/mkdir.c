/*
 * mkdir() wrapper.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *pathname, mode_t mode
#define WRAPPER_ARGS pathname, mode
#define WRAPPER_SAFE() SB_SAFE(pathname)
#include "mkdirat.c"
