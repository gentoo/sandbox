/*
 * creat64() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

#define WRAPPER_ARGS const char *pathname, mode_t mode
extern int EXTERN_NAME(WRAPPER_ARGS);
/* XXX: We use the open64() call to simulate creat64() */
/* static int (*WRAPPER_TRUE_NAME)(WRAPPER_ARGS) = NULL; */

int WRAPPER_NAME(WRAPPER_ARGS)
{
	int result = -1;

	if (FUNCTION_SANDBOX_SAFE(pathname)) {
		check_dlsym(true_open64_DEFAULT, symname_open64_DEFAULT,
			    symver_open64_DEFAULT);
		result = true_open64_DEFAULT(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
	}

	return result;
}
