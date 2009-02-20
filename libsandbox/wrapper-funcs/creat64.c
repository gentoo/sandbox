/*
 * creat64() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define sb_unwrapped_open_DEFAULT sb_unwrapped_open64_DEFAULT
#include "creat.c"
#undef sb_unwrapped_open_DEFAULT
