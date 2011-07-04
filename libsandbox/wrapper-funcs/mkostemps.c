/*
 * mkostemps() wrapper.
 *
 * Copyright 1999-2011 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO char *template, int suffixlen, int flags
#define WRAPPER_ARGS template, suffixlen, flags
#define WRAPPER_SAFE() SB_SAFE(template)
#include "__wrapper_simple.c"
