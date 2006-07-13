/*
 * is_env_on.c
 *
 * Util functions.
 *
 * Copyright 1999-2006 Gentoo Foundation
 *
 *
 *      This program is free software; you can redistribute it and/or modify it
 *      under the terms of the GNU General Public License as published by the
 *      Free Software Foundation version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Header$
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

