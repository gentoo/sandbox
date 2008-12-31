/*
 * getcwd() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

#define WRAPPER_ARGS_PROTO char *buf, size_t size
#define WRAPPER_ARGS dirfd, filename, times
extern char *WRAPPER_NAME(WRAPPER_ARGS_PROTO);
static char *(*WRAPPER_TRUE_NAME)(WRAPPER_ARGS_PROTO) = NULL;

char *WRAPPER_NAME(WRAPPER_ARGS_PROTO)
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
