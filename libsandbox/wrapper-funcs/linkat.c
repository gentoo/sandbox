/*
 * linkat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS int olddirfd, const char *oldpath, int newdirfd, const char *newpath, int flags
extern int EXTERN_NAME(WRAPPER_ARGS);
static int (*WRAPPER_TRUE_NAME)(WRAPPER_ARGS) = NULL;

int WRAPPER_NAME(WRAPPER_ARGS)
{
	int result = -1;

	if FUNCTION_SANDBOX_SAFE_AT(newdirfd, STRING_NAME, newpath) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(olddirfd, oldpath, newdirfd, newpath, flags);
	}

	return result;
}
