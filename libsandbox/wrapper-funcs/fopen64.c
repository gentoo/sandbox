/*
 * fopen64() wrapper.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define sb_fopen_pre_check sb_fopen64_pre_check
#include "fopen.c"
#undef sb_fopen_pre_check
