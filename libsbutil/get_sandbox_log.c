/*
 * get_sandbox_log.c
 *
 * Util functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 * Some parts might have Copyright:
 *   Copyright (C) 2002 Brad House <brad@mainstreetsoftworks.com>
 */

#include "headers.h"
#include "sbutil.h"

static void _get_sb_log(char *path, const char *tmpdir, const char *env, const char *prefix)
{
	char *sandbox_log_env = NULL;

	save_errno();

	sandbox_log_env = getenv(env);

	if (sandbox_log_env && is_env_on(ENV_SANDBOX_TESTING)) {
		/* When testing, just use what the env says to */
		strncpy(path, sandbox_log_env, SB_PATH_MAX);
	} else {
		/* THIS CHUNK BREAK THINGS BY DOING THIS:
		 * SANDBOX_LOG=/tmp/sandbox-app-admin/superadduser-1.0.7-11063.log
		 */
		if ((NULL != sandbox_log_env) &&
		    (NULL != strchr(sandbox_log_env, '/')))
		    sandbox_log_env = NULL;

		/* If running as a user w/out write access to /var/log, don't
		 * shit ourselves.
		 */
		const char *sb_log_dir = SANDBOX_LOG_LOCATION;
		if (tmpdir && access(sb_log_dir, W_OK))
			sb_log_dir = tmpdir;

		snprintf(path, SB_PATH_MAX, "%s%s%s%s%d%s",
			sb_log_dir, prefix,
			(sandbox_log_env == NULL ? "" : sandbox_log_env),
			(sandbox_log_env == NULL ? "" : "-"),
			getpid(), LOG_FILE_EXT);
	}

	restore_errno();
}

void get_sandbox_log(char *path, const char *tmpdir)
{
	_get_sb_log(path, tmpdir, ENV_SANDBOX_LOG, LOG_FILE_PREFIX);
}

void get_sandbox_debug_log(char *path, const char *tmpdir)
{
	_get_sb_log(path, tmpdir, ENV_SANDBOX_DEBUG_LOG, DEBUG_LOG_FILE_PREFIX);
}

void get_sandbox_message_path(char *path)
{
	_get_sb_log(path, NULL, ENV_SANDBOX_MESSAGE_PATH, DEBUG_LOG_FILE_PREFIX);
}
