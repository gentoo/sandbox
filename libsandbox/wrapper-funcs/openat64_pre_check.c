/*
 * open*64*() pre-check.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "__64_pre.h"
#define sb_openat_pre_check sb_openat64_pre_check
#include "openat_pre_check.c"
#undef sb_openat_pre_check
#include "__64_post.h"
