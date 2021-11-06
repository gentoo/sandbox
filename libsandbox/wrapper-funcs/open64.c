/*
 * open64() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define sb_openat_pre_check sb_openat64_pre_check
#include "open.c"
#undef sb_openat_pre_check
