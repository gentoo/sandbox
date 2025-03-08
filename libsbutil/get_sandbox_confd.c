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
	char *p;
	if (is_env_on(ENV_SANDBOX_TESTING) && (p = getenv("__SANDBOX_CONFD_DIR")))
		strlcpy(path, p, PATH_MAX);
	else
		strcpy(path, SANDBOX_CONFD_DIR);
	restore_errno();
	return path;
}
