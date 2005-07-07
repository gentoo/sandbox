/*
 * Copyright (C) 2002 Brad House <brad@mainstreetsoftworks.com>
 * Distributed under the terms of the GNU General Public License, v2 or later 
 * Author: Brad House <brad@mainstreetsoftworks.com>
 *
 * $Header$
 * 
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <libgen.h>

#include <grp.h>
#include <pwd.h>

#include "sandbox.h"
#include "config.h"

/* glibc modified getcwd() functions */
SB_STATIC char *egetcwd(char *, size_t);

SB_STATIC void get_sandbox_path(char *argv0, char *path)
{
	memset(path, 0, sizeof(path));
	/* ARGV[0] specifies full path */
	if ((NULL != argv0) && (argv0[0] == '/')) {
		snprintf(path, SB_PATH_MAX, "%s", argv0);

		/* ARGV[0] specifies relative path */
	} else {
		if (-1 != readlink("/proc/self/exe", path, SB_PATH_MAX))
			path[SB_PATH_MAX - 1] = '\0';
		else
			path[0] = '\0';
	}

	/* Return just directory */
	dirname(path);
}

SB_STATIC void get_sandbox_lib(char *path)
{
#ifdef SB_HAVE_64BIT_ARCH
	snprintf(path, SB_PATH_MAX, "%s", LIB_NAME);
#else
	snprintf(path, SB_PATH_MAX, "%s/%s", LIBSANDBOX_PATH, LIB_NAME);
	if (0 >= exists(path)) {
		snprintf(path, SB_PATH_MAX, "%s", LIB_NAME);
	}
#endif
}

SB_STATIC void get_sandbox_rc(char *path)
{
	char sb_path[SB_PATH_MAX];

	snprintf(path, SB_PATH_MAX, "%s/%s", SANDBOX_BASHRC_PATH, BASHRC_NAME);
	if (0 >= exists(path)) {
		get_sandbox_path(NULL, sb_path);
		snprintf(path, SB_PATH_MAX, "%s%s", sb_path, BASHRC_NAME);
	}
}

SB_STATIC void get_sandbox_log(char *path)
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

SB_STATIC void get_sandbox_debug_log(char *path)
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

SB_STATIC int get_tmp_dir(char *path)
{
	if (NULL == realpath(getenv(ENV_TMPDIR) ? getenv(ENV_TMPDIR)
					      : TMPDIR,
				path)) {
		if (NULL == realpath(TMPDIR, path))
			return -1;
	}

	return 0;
}

SB_STATIC int exists(const char *pathname)
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

SB_STATIC int is_file(const char *pathname)
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

SB_STATIC long file_length(int fd)
{
	long pos, len;

	pos = lseek(fd, 0L, SEEK_CUR);
	len = lseek(fd, 0L, SEEK_END);
	lseek(fd, pos, SEEK_SET);

	return (len);
}


// vim:noexpandtab noai:cindent ai
