/*
 * generic wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO
# error WRAPPER_ARGS_PROTO needs to be defined
#endif
#ifndef WRAPPER_RET_TYPE
# define WRAPPER_RET_TYPE int
#endif
#ifndef WRAPPER_RET_DEFAULT
# define WRAPPER_RET_DEFAULT -1
#endif
#ifndef WRAPPER_PRE_CHECKS
# define WRAPPER_PRE_CHECKS() true
#endif

extern WRAPPER_RET_TYPE EXTERN_NAME(WRAPPER_ARGS_PROTO);
static WRAPPER_RET_TYPE (*WRAPPER_TRUE_NAME)(WRAPPER_ARGS_PROTO) = NULL;

WRAPPER_RET_TYPE WRAPPER_NAME(WRAPPER_ARGS_PROTO)
{
	WRAPPER_RET_TYPE result = WRAPPER_RET_DEFAULT;

	if (WRAPPER_PRE_CHECKS())
		if (WRAPPER_SAFE()) {
			check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME, WRAPPER_SYMVER);
			result = WRAPPER_TRUE_NAME(WRAPPER_ARGS);
		}

	return result;
}
