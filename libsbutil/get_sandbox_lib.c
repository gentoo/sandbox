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

/* Always let the dynamic loader do the searching rather than hard coding the
 * full path.  This way, things like multilib, testing, local runs work easier.
 */
void get_sandbox_lib(char *path)
{
	save_errno();
	snprintf(path, SB_PATH_MAX, "%s", LIB_NAME);
	restore_errno();
}
