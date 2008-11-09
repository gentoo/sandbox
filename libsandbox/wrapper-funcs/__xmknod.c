/*
 * __xmknod.c
 *
 * __xmknod() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

extern int EXTERN_NAME(int, const char *, __mode_t, __dev_t *);
static int (*WRAPPER_TRUE_NAME) (int, const char *, __mode_t, __dev_t *) = NULL;

int WRAPPER_NAME(int ver, const char *pathname, __mode_t mode, __dev_t *dev)
{
	int result = -1;

	if FUNCTION_SANDBOX_SAFE("mknod", pathname) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(ver, pathname, mode, dev);
	}

	return result;
}
