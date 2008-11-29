/*
 * opendir() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS const char *name
extern DIR *WRAPPER_NAME(WRAPPER_ARGS);
static DIR *(*WRAPPER_TRUE_NAME)(WRAPPER_ARGS) = NULL;

DIR *WRAPPER_NAME(WRAPPER_ARGS)
{
	DIR *result = NULL;

	if (FUNCTION_SANDBOX_SAFE(name)) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(name);
	}

	return result;
}
