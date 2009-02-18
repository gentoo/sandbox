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
#define WRAPPER_ARGS buf, size
#define WRAPPER_RET_TYPE char *
#define WRAPPER_RET_DEFAULT NULL

extern WRAPPER_RET_TYPE EXTERN_NAME(WRAPPER_ARGS_PROTO);
static WRAPPER_RET_TYPE (*WRAPPER_TRUE_NAME)(WRAPPER_ARGS_PROTO) = NULL;

attribute_hidden
WRAPPER_RET_TYPE SB_HIDDEN_FUNC(WRAPPER_NAME)(WRAPPER_ARGS_PROTO)
{
	check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME, WRAPPER_SYMVER);
	return WRAPPER_TRUE_NAME(WRAPPER_ARGS);
}

WRAPPER_RET_TYPE WRAPPER_NAME(WRAPPER_ARGS_PROTO)
{
	WRAPPER_RET_TYPE result = WRAPPER_RET_DEFAULT;

	/* Need to disable sandbox, as on non-linux libc's, opendir() is
	 * used by some getcwd() implementations and resolves to the sandbox
	 * opendir() wrapper, causing infinit recursion and finially crashes.
	 */
	sandbox_on = false;
	result = SB_HIDDEN_FUNC(WRAPPER_NAME)(WRAPPER_ARGS);
	sandbox_on = true;

	return result;
}
