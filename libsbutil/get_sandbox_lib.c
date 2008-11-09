/*
 * get_sandbox_lib.c
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

void get_sandbox_lib(char *path)
{
	save_errno();
	snprintf(path, SB_PATH_MAX, "%s/%s", LIBSANDBOX_PATH, LIB_NAME);
	if (!rc_file_exists(path)) {
		snprintf(path, SB_PATH_MAX, "%s", LIB_NAME);
	}
	restore_errno();
}
