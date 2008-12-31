/*
 * openat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO /* let open() use us */
# define WRAPPER_ARGS_PROTO int dirfd, const char *pathname, int flags, ...
# define WRAPPER_ARGS dirfd, pathname, flags
# define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_OPEN_INT_AT(dirfd, pathname, flags)
# define USE_AT 1
#else
# define USE_AT 0
#endif

extern int EXTERN_NAME(WRAPPER_ARGS_PROTO);
static int (*WRAPPER_TRUE_NAME)(WRAPPER_ARGS_PROTO) = NULL;

/* Eventually, there is a third parameter: it's mode_t mode */
int WRAPPER_NAME(WRAPPER_ARGS_PROTO)
{
	va_list ap;
	int mode = 0;
	int result = -1;

	save_errno();
	if (flags & O_CREAT) {
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
	} else {
		/* XXX: If we're not trying to create, fail normally if
		 *      file does not stat */
		struct stat st;
#if USE_AT
		if (dirfd == AT_FDCWD || pathname[0] == '/')
#endif
#undef USE_AT
			if (-1 == stat(pathname, &st))
				return -1;
	}
	restore_errno();

	if (WRAPPER_SAFE()) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		if (flags & O_CREAT)
			result = WRAPPER_TRUE_NAME(WRAPPER_ARGS, mode);
		else
			result = WRAPPER_TRUE_NAME(WRAPPER_ARGS);
	}

	return result;
}
