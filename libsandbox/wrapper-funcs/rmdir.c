/*
 * rmdir() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS const char *pathname
extern int EXTERN_NAME(WRAPPER_ARGS);
static int (*WRAPPER_TRUE_NAME)(WRAPPER_ARGS) = NULL;

int WRAPPER_NAME(WRAPPER_ARGS)
{
	int result = -1;

	if FUNCTION_SANDBOX_SAFE(STRING_NAME, pathname) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(pathname);
	}

	return result;
}
