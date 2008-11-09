/*
 * getcwd.c
 *
 * getcwd() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

extern char *WRAPPER_NAME(char *, size_t);
static char * (*WRAPPER_TRUE_NAME) (char *, size_t) = NULL;

char *WRAPPER_NAME(char *buf, size_t size)
{
	char *result = NULL;

	/* Need to disable sandbox, as on non-linux libc's, opendir() is
	 * used by some getcwd() implementations and resolves to the sandbox
	 * opendir() wrapper, causing infinit recursion and finially crashes.
	 */
	sandbox_on = 0;
	check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
	result = WRAPPER_TRUE_NAME(buf, size);
	sandbox_on = 1;

	return result;
}
