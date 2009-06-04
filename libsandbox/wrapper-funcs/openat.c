/*
 * openat() wrapper.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO /* let open() use us */
# define _WRAPPER_ARGS_PROTO int dirfd, const char *pathname, int flags
# define WRAPPER_ARGS_PROTO _WRAPPER_ARGS_PROTO, ...
# define WRAPPER_ARGS_PROTO_FULL _WRAPPER_ARGS_PROTO, mode_t mode
# define WRAPPER_ARGS dirfd, pathname, flags
# define WRAPPER_ARGS_FULL WRAPPER_ARGS, mode
# define WRAPPER_SAFE() SB_SAFE_OPEN_INT_AT(dirfd, pathname, flags)
#else
# define dirfd AT_FDCWD
#endif

#ifdef SB64
# define WRAPPER_PRE_CHECKS() sb_openat64_pre_check(STRING_NAME, pathname, dirfd, flags)
#else
# define WRAPPER_PRE_CHECKS() sb_openat_pre_check(STRING_NAME, pathname, dirfd, flags)
#endif

#define WRAPPER_SAFE_POST_EXPAND \
	int mode = 0; \
	if (flags & O_CREAT) { \
		va_list ap; \
		va_start(ap, flags); \
		mode = va_arg(ap, int); \
		va_end(ap); \
	}

#include "__wrapper_simple.c"

#undef dirfd
#undef _WRAPPER_ARGS_PROTO
