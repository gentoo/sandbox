/*
 * fopen() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

#define WRAPPER_ARGS const char *pathname, const char *mode
extern FILE *WRAPPER_NAME(WRAPPER_ARGS);
static FILE *(*WRAPPER_TRUE_NAME)(WRAPPER_ARGS) = NULL;

FILE *WRAPPER_NAME(WRAPPER_ARGS)
{
	FILE *result = NULL;
	int old_errno = errno;
	struct stat st;

	if ((NULL != mode) && (mode[0] == 'r')) {
		/* XXX: If we're trying to read, fail normally if file does
		 *      not stat */
		if (-1 == stat(pathname, &st))
			return NULL;
	}
	errno = old_errno;

	if FUNCTION_SANDBOX_SAFE_OPEN_CHAR(STRING_NAME, pathname, mode) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(pathname, mode);
	}

	return result;
}
