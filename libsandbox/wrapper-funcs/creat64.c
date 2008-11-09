/*
 * creat64.c
 *
 * creat64() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

extern int EXTERN_NAME(const char *, __mode_t);
/* XXX: We use the open64() call to simulate create64() */
/* static int (*WRAPPER_TRUE_NAME) (const char *, __mode_t) = NULL; */

int WRAPPER_NAME(const char *pathname, __mode_t mode)
{
	int result = -1;

	if FUNCTION_SANDBOX_SAFE("creat64", pathname) {
		check_dlsym(true_open64_DEFAULT, symname_open64_DEFAULT,
			    symver_open64_DEFAULT);
		result = true_open64_DEFAULT(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
	}

	return result;
}
