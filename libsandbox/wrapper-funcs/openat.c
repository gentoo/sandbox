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
#define WRAPPER_RET_TYPE int
#define WRAPPER_RET_DEFAULT -1

extern WRAPPER_RET_TYPE EXTERN_NAME(WRAPPER_ARGS_PROTO);
static WRAPPER_RET_TYPE (*WRAPPER_TRUE_NAME)(WRAPPER_ARGS_PROTO) = NULL;

attribute_hidden
WRAPPER_RET_TYPE SB_HIDDEN_FUNC(WRAPPER_NAME)(WRAPPER_ARGS_PROTO_FULL)
{
	check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME, WRAPPER_SYMVER);
	return WRAPPER_TRUE_NAME(WRAPPER_ARGS_FULL);
}

/* Eventually, there is a third parameter: it's mode_t mode */
WRAPPER_RET_TYPE WRAPPER_NAME(WRAPPER_ARGS_PROTO)
{
	WRAPPER_RET_TYPE result = WRAPPER_RET_DEFAULT;

	if (!(flags & O_CREAT)) {
		save_errno();

		/* XXX: If we're not trying to create, fail normally if
		 *      file does not stat */
		struct stat st;
#if USE_AT
		if (dirfd == AT_FDCWD || pathname[0] == '/')
#endif
#undef USE_AT
			if (-1 == stat(pathname, &st))
				return -1;

		restore_errno();
	}

	if (WRAPPER_SAFE()) {
		int mode = 0;
		if (flags & O_CREAT) {
			va_list ap;
			va_start(ap, flags);
			mode = va_arg(ap, int);
			va_end(ap);
		}
		result = SB_HIDDEN_FUNC(WRAPPER_NAME)(WRAPPER_ARGS_FULL);
	}

	return result;
}

#undef _WRAPPER_ARGS_PROTO
#undef WRAPPER_ARGS_PROTO_FULL
#undef WRAPPER_ARGS_FULL
