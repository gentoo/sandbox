/*
 * mkfifoat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int dirfd, const char *pathname, mode_t mode
#define WRAPPER_ARGS dirfd, pathname, mode
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_AT(dirfd, pathname, 0)
#include "__wrapper_simple.c"
