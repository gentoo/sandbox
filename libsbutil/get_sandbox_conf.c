/*
 * get_sandbox_conf.c
 *
 * Util functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

/* Where the file exists in our source tree */
#define LOCAL_SANDBOX_CONF_FILE "/etc/sandbox.conf"

char *get_sandbox_conf(void)
{
	char *ret = SANDBOX_CONF_FILE;
	save_errno();
	if (is_env_on(ENV_SANDBOX_TESTING)) {
		char *abs = getenv("abs_top_srcdir");
		ret = xmalloc(strlen(abs) + strlen(LOCAL_SANDBOX_CONF_FILE) + 1);
		sprintf(ret, "%s%s", abs, LOCAL_SANDBOX_CONF_FILE);
	}
	restore_errno();
	return ret;
}
