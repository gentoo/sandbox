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

char *get_sandbox_conf(void)
{
	char *ret = SANDBOX_CONF_FILE;
	save_errno();
	if (is_env_on(ENV_SANDBOX_TESTING))
		ret = getenv("__SANDBOX_CONF_FILE");
	restore_errno();
	return ret;
}
