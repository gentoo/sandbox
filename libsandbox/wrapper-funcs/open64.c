/*
 * open64.c
 *
 * open64() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

extern int EXTERN_NAME(const char *, int, ...);
static int (*WRAPPER_TRUE_NAME) (const char *, int, ...) = NULL;

/* Eventually, there is a third parameter: it's mode_t mode */
int WRAPPER_NAME(const char *pathname, int flags, ...)
{
	va_list ap;
	int mode = 0;
	int result = -1;
	int old_errno = errno;
	struct stat64 st;

	if (flags & O_CREAT) {
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
	} else {
		/* XXX: If we're not trying to create, fail normally if
		 *      file does not stat */
		if (-1 == stat64(pathname, &st)) {
			return -1;
		}
	}
	errno = old_errno;

	if FUNCTION_SANDBOX_SAFE_OPEN_INT("open64", pathname, flags) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		if (flags & O_CREAT)
			result = WRAPPER_TRUE_NAME(pathname, flags, mode);
		else
			result = WRAPPER_TRUE_NAME(pathname, flags);
	}

	return result;
}
