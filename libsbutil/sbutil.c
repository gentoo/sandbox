/*
 * sbutil.c
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
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>

#include "sbutil.h"

void get_sandbox_lib(char *path)
{
#ifdef SB_HAVE_MULTILIB
	snprintf(path, SB_PATH_MAX, "%s", LIB_NAME);
#else
	snprintf(path, SB_PATH_MAX, "%s/%s", LIBSANDBOX_PATH, LIB_NAME);
	if (!rc_file_exists(path)) {
		snprintf(path, SB_PATH_MAX, "%s", LIB_NAME);
	}
#endif
}

void get_sandbox_rc(char *path)
{
	snprintf(path, SB_PATH_MAX, "%s/%s", SANDBOX_BASHRC_PATH, BASHRC_NAME);
}

void get_sandbox_log(char *path)
{
	char *sandbox_log_env = NULL;

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
}

void get_sandbox_debug_log(char *path)
{
	char *sandbox_debug_log_env = NULL;

	sandbox_debug_log_env = getenv(ENV_SANDBOX_DEBUG_LOG);
	
	/* THIS CHUNK BREAK THINGS BY DOING THIS:
	 * SANDBOX_DEBUG_LOG=/tmp/sandbox-app-admin/superadduser-1.0.7-11063.log
	 */
	if ((NULL != sandbox_debug_log_env) &&
	    (NULL != strchr(sandbox_debug_log_env, '/')))
		sandbox_debug_log_env = NULL;

	snprintf(path, SB_PATH_MAX, "%s%s%s%s%d%s",
			SANDBOX_LOG_LOCATION, DEBUG_LOG_FILE_PREFIX,
			(sandbox_debug_log_env == NULL ? "" : sandbox_debug_log_env),
			(sandbox_debug_log_env == NULL ? "" : "-"),
			getpid(), LOG_FILE_EXT);
}

int get_tmp_dir(char *path)
{
	if (NULL == realpath(getenv(ENV_TMPDIR) ? getenv(ENV_TMPDIR)
					      : TMPDIR,
				path)) {
		if (NULL == realpath(TMPDIR, path))
			return -1;
	}

	/* Reset errno in case realpath set it */
	errno = 0;
		
	return 0;
}

bool is_env_on (const char *env)
{
	if ((NULL != env) && (NULL != getenv(env)) &&
	    ((0 == strncasecmp(getenv(env), "1", 1)) ||
	     (0 == strncasecmp(getenv(env), "true", 4)) ||
	     (0 == strncasecmp(getenv(env), "yes", 3))))
		return TRUE;
		
	return FALSE;
}

bool is_env_off (const char *env)
{
	if ((NULL != env) && (NULL != getenv(env)) &&
	    ((0 == strncasecmp(getenv(env), "0", 1)) ||
	     (0 == strncasecmp(getenv(env), "false", 5)) ||
	     (0 == strncasecmp(getenv(env), "no", 2))))
		return TRUE;
		
	return FALSE;
}


// vim:noexpandtab noai:cindent ai
