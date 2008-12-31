/*
 * get_tmp_dir.c
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

int get_tmp_dir(char *path)
{
	save_errno();

	if (NULL == realpath(getenv(ENV_TMPDIR) ? : TMPDIR, path))
		if (NULL == realpath(TMPDIR, path))
			return -1;

	restore_errno();

	return 0;
}
