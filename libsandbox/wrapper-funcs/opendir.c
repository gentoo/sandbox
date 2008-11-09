/*
 * opendir.c
 *
 * opendir() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

extern DIR *WRAPPER_NAME(const char *);
static DIR * (*WRAPPER_TRUE_NAME) (const char *) = NULL;

DIR *WRAPPER_NAME(const char *name)
{
	DIR *result = NULL;

	if FUNCTION_SANDBOX_SAFE("opendir", name) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(name);
	}

	return result;
}
