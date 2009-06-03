/*
 * fopen64() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "__64_pre.h"
#undef SB_FOPEN_PRE_CHECK
#define sb_fopen_pre_check sb_fopen64_pre_check
#define WRAPPER_PRE_CHECKS() sb_fopen64_pre_check(WRAPPER_ARGS)
#include "fopen.c"
#undef sb_fopen_pre_check
#include "__64_post.h"
