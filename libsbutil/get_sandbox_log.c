/*
 * get_sandbox_log.c
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
 * Some parts might have Copyright:
 *
 *   Copyright (C) 2002 Brad House <brad@mainstreetsoftworks.com>
 *
 * $Header$
 */


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "sbutil.h"

void get_sandbox_log(char *path)
{
	char *sandbox_log_env = NULL;

	save_errno();

	sandbox_log_env = getenv(ENV_SANDBOX_LOG);
	
	/* THIS CHUNK BREAK THINGS BY DOING THIS:
	 * SANDBOX_LOG=/tmp/sandbox-app-admin/superadduser-1.0.7-11063.log
	 */
	if ((NULL != sandbox_log_env) &&
	    (NULL != strchr(sandbox_log_env, '/')))
	    sandbox_log_env = NULL;

	snprintf(path, SB_PATH_MAX, "%s%s%s%s%d%s",
			SANDBOX_LOG_LOCATION, LOG_FILE_PREFIX,
			(sandbox_log_env == NULL ? "" : sandbox_log_env),
			(sandbox_log_env == NULL ? "" : "-"),
			getpid(), LOG_FILE_EXT);

	restore_errno();
}

