/*
 * get_sandbox_confd.c
 *
 * Util functions.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

char *get_sandbox_confd(char *path)
{
	save_errno();
	if (is_env_on(ENV_SANDBOX_TESTING))
		snprintf(path, SB_PATH_MAX, "%s/etc/sandbox.d/",
			getenv("abs_top_srcdir"));
	else
		strcpy(path, SANDBOX_CONFD_DIR);
	restore_errno();
	return path;
}
