/*
 * openat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO /* let open() use us */
# define _WRAPPER_ARGS_PROTO int dirfd, const char *pathname, int flags
# define WRAPPER_ARGS_PROTO _WRAPPER_ARGS_PROTO, ...
# define WRAPPER_ARGS_PROTO_FULL _WRAPPER_ARGS_PROTO, mode_t mode
# define WRAPPER_ARGS dirfd, pathname, flags
# define WRAPPER_ARGS_FULL WRAPPER_ARGS, mode
# define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_OPEN_INT_AT(dirfd, pathname, flags)
# define USE_AT 1
#else
# define USE_AT 0
#endif

#ifndef PRE_CHECK_FUNC
# define _PRE_CHECK_FUNC(x) sb_##x##_pre_check
# define PRE_CHECK_FUNC(x)  _PRE_CHECK_FUNC(x)
#endif

static inline bool PRE_CHECK_FUNC(WRAPPER_NAME)(_WRAPPER_ARGS_PROTO)
{
	if (!(flags & O_CREAT)) {
		/* If we're not trying to create, fail normally if
		 * file does not stat
		 */
#if USE_AT
		if (dirfd == AT_FDCWD || pathname[0] == '/')
#endif
#undef USE_AT
		{
			struct stat st;
			save_errno();
			if (-1 == stat(pathname, &st)) {
				if (is_env_on(ENV_SANDBOX_DEBUG))
					SB_EINFO("EARLY FAIL", "  %s(%s): %s\n",
						STRING_NAME, pathname, strerror(errno));
				return false;
			}
			restore_errno();
		}
	}

	return true;
}
#define WRAPPER_PRE_CHECKS() PRE_CHECK_FUNC(WRAPPER_NAME)(WRAPPER_ARGS)

#define WRAPPER_SAFE_POST_EXPAND \
	int mode = 0; \
	if (flags & O_CREAT) { \
		va_list ap; \
		va_start(ap, flags); \
		mode = va_arg(ap, int); \
		va_end(ap); \
	}

#include "__wrapper_simple.c"

#undef _WRAPPER_ARGS_PROTO
