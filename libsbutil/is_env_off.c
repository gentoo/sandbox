/*
 * is_env_off.c
 *
 * Util functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "sbutil.h"

bool is_env_off (const char *env)
{
	if ((NULL != env) && (NULL != getenv(env)) &&
	    ((0 == strncasecmp(getenv(env), "0", 1)) ||
	     (0 == strncasecmp(getenv(env), "false", 5)) ||
	     (0 == strncasecmp(getenv(env), "no", 2))))
		return TRUE;

	return FALSE;
}
