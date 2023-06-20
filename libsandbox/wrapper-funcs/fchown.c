/*
 * fchown() wrapper.
 *
 * Copyright 1999-2018 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int fd, uid_t owner, gid_t group
#define WRAPPER_ARGS fd, owner, group
#define WRAPPER_SAFE() SB_SAFE_FD(fd)
#include "__wrapper_simple.c"
