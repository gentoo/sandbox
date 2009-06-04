/*
 * fopen64() pre-check.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "__64_pre.h"
#define sb_fopen_pre_check sb_fopen64_pre_check
#include "fopen_pre_check.c"
#undef sb_fopen_pre_check
#include "__64_post.h"
