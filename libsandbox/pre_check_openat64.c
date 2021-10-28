/*
 * open*64*() pre-check.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"
#include "libsandbox.h"
#include "wrappers.h"

#include "wrapper-funcs/__64_pre.h"
#define sb_openat_pre_check sb_openat64_pre_check
#include "pre_check_openat.c"
#undef sb_openat_pre_check
#include "wrapper-funcs/__64_post.h"
