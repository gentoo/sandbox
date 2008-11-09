/*
 * rmdir.c
 *
 * rmdir() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

extern int EXTERN_NAME(const char *);
static int (*WRAPPER_TRUE_NAME) (const char *) = NULL;

int WRAPPER_NAME(const char *pathname)
{
	int result = -1;

	if FUNCTION_SANDBOX_SAFE("rmdir", pathname) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(pathname);
	}

	return result;
}
