/*
 * open() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

#define WRAPPER_ARGS const char *pathname, int flags, ...
extern int EXTERN_NAME(WRAPPER_ARGS);
static int (*WRAPPER_TRUE_NAME)(WRAPPER_ARGS) = NULL;

/* Eventually, there is a third parameter: it's mode_t mode */
int WRAPPER_NAME(WRAPPER_ARGS)
{
	va_list ap;
	int mode = 0;
	int result = -1;
	int old_errno = errno;
	struct stat st;

	if (flags & O_CREAT) {
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
	} else {
		/* XXX: If we're not trying to create, fail normally if
		 *      file does not stat */
		if (-1 == stat(pathname, &st))
			return -1;
	}
	errno = old_errno;

	if (FUNCTION_SANDBOX_SAFE_OPEN_INT(pathname, flags)) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		if (flags & O_CREAT)
			result = WRAPPER_TRUE_NAME(pathname, flags, mode);
		else
			result = WRAPPER_TRUE_NAME(pathname, flags);
	}

	return result;
}
