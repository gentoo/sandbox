/*
 * chown() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *path, uid_t owner, gid_t group
#define WRAPPER_ARGS path, owner, group
#define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE(path)
#include "__wrapper_simple.c"
