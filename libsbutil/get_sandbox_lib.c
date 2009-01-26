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
 *
 * Make an exception for non-standard setups (i.e. prefix) where libsandbox is
 * installed into a path that is not in ld.so.conf.
 */
void get_sandbox_lib(char *path)
{
	save_errno();
	strcpy(path, LIB_NAME);
	if (strncmp("/usr/lib", LIBSANDBOX_PATH, 8)) {
		void *hndl = dlopen(path, RTLD_LAZY);
		if (!hndl)
			snprintf(path, SB_PATH_MAX, "%s/%s", LIBSANDBOX_PATH, LIB_NAME);
		else
			dlclose(hndl);
	}
	restore_errno();
}
