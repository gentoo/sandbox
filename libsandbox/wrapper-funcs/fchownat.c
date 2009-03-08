/*
 * fchownat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int dirfd, const char *path, uid_t owner, gid_t group, int flags
#define WRAPPER_ARGS dirfd, path, owner, group, flags
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_AT(dirfd, path, flags)
#include "__wrapper_simple.c"
