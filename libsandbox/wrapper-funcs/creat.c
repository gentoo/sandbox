/*
 * creat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

#define WRAPPER_ARGS_PROTO const char *pathname, mode_t mode
#define WRAPPER_ARGS pathname, mode
extern int EXTERN_NAME(WRAPPER_ARGS_PROTO);
/* XXX: We use the open() call to simulate creat() */
/* static int (*WRAPPER_TRUE_NAME)(WRAPPER_ARGS_PROTO) = NULL; */

int WRAPPER_NAME(WRAPPER_ARGS_PROTO)
{
	int result = -1;

	if (FUNCTION_SANDBOX_SAFE(pathname))
		result = sb_unwrapped_open_DEFAULT(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);

	return result;
}
