/*
 * Copyright (C) 2002 Brad House <brad@mainstreetsoftworks.com>
 * Distributed under the terms of the GNU General Public License, v2 or later 
 * Author: Brad House <brad@mainstreetsoftworks.com>
 * Author: Martin Schlemmer <azarah@gentoo.org>
 *
 * $Header$
 * 
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

#include "config.h"
#include "localdecls.h"

#include "sandbox.h"

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

#ifdef OUTSIDE_LIBSANDBOX

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

	return 0;
}

long file_length(int fd)
{
	struct stat st;
	int retval;

	retval = fstat(fd, &st);
	if (-1 == retval)
		return 0;

	return (st.st_size);
}

#endif /* OUTSIDE_LIBSANDBOX */

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
