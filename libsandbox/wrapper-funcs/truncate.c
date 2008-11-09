/*
 * truncate.c
 *
 * truncate() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

extern int EXTERN_NAME(const char *, TRUNCATE_T);
static int (*WRAPPER_TRUE_NAME) (const char *, TRUNCATE_T) = NULL;

int WRAPPER_NAME(const char *path, TRUNCATE_T length)
{
	int result = -1;

	if FUNCTION_SANDBOX_SAFE("truncate", path) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(path, length);
	}

	return result;
}