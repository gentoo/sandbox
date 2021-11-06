/*
 * __openat64_2() wrapper (_FORTIFY_SOURCE).
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define sb_openat_pre_check sb_openat64_pre_check
#include "__openat_2.c"
#undef sb_openat_pre_check
