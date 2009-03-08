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

static void _get_sb_log(char *path, const char *env, const char *prefix)
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

		snprintf(path, SB_PATH_MAX, "%s%s%s%s%d%s",
			SANDBOX_LOG_LOCATION, prefix,
			(sandbox_log_env == NULL ? "" : sandbox_log_env),
			(sandbox_log_env == NULL ? "" : "-"),
			getpid(), LOG_FILE_EXT);
	}

	restore_errno();
}

void get_sandbox_log(char *path)
{
	_get_sb_log(path, ENV_SANDBOX_LOG, LOG_FILE_PREFIX);
}

void get_sandbox_debug_log(char *path)
{
	_get_sb_log(path, ENV_SANDBOX_DEBUG_LOG, DEBUG_LOG_FILE_PREFIX);
}
