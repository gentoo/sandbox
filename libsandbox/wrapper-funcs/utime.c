/*
 * utime.c
 *
 * utime() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

extern int EXTERN_NAME(const char *, const struct utimbuf *);
static int (*WRAPPER_TRUE_NAME) (const char *, const struct utimbuf *) = NULL;

int WRAPPER_NAME(const char *filename, const struct utimbuf *times)
{
	int result = -1;

	if FUNCTION_SANDBOX_SAFE("utime", filename) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(filename, times);
	}

	return result;
}
