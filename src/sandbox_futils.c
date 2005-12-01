/*
 * Copyright (C) 2002 Brad House <brad@mainstreetsoftworks.com>
 * Distributed under the terms of the GNU General Public License, v2 or later 
 * Author: Brad House <brad@mainstreetsoftworks.com>
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
	if (0 >= exists(path)) {
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

#endif /* OUTSIDE_LIBSANDBOX */

int exists(const char *pathname)
{
	struct stat buf;
	int retval;

	if ((NULL == pathname) || (0 == strlen(pathname)))
		return 0;

	retval = lstat(pathname, &buf);
	if (-1 != retval)
		return 1;
	/* Some or other error occurred */
	if (ENOENT != errno)
		return -1;

	return 0;
}

#ifdef OUTSIDE_LIBSANDBOX

int is_file(const char *pathname)
{
	struct stat buf;
	int retval;

	if ((NULL == pathname) || (0 == strlen(pathname)))
		return 0;

	retval = lstat(pathname, &buf);
	if ((-1 != retval) && (S_ISREG(buf.st_mode)))
		return 1;
	/* Some or other error occurred */
	if ((-1 == retval) && (ENOENT != errno))
		return -1;

	return 0;
}

int is_dir(const char *pathname, int follow_link)
{
	struct stat buf;
	int retval;

	if ((NULL == pathname) || (0 == strlen(pathname)))
		return 0;

	retval = follow_link ? stat(pathname, &buf) : lstat(pathname, &buf);
	if ((-1 != retval) && (S_ISDIR(buf.st_mode)))
		return 1;
	/* Some or other error occurred */
	if ((-1 == retval) && (ENOENT != errno))
		return -1;

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


// vim:noexpandtab noai:cindent ai
