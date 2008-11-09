/*
 * is_env_on.c
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

bool is_env_on (const char *env)
{
	if ((NULL != env) && (NULL != getenv(env)) &&
	    ((0 == strncasecmp(getenv(env), "1", 1)) ||
	     (0 == strncasecmp(getenv(env), "true", 4)) ||
	     (0 == strncasecmp(getenv(env), "yes", 3))))
		return TRUE;

	return FALSE;
}
