/*
 * sandbox.h
 *
 * Main sandbox related functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 * Some parts might have Copyright:
 *   Copyright (C) 2002 Brad House <brad@mainstreetsoftworks.com>
 */

#ifndef __SANDBOX_H__
#define __SANDBOX_H__

struct sandbox_info_t {
	char sandbox_log[SB_PATH_MAX];
	char sandbox_debug_log[SB_PATH_MAX];
	char sandbox_lib[SB_PATH_MAX];
	char sandbox_rc[SB_PATH_MAX];
	char work_dir[SB_PATH_MAX];
	char tmp_dir[SB_PATH_MAX];
	char *home_dir;
} sandbox_info_t;

extern char **setup_environ(struct sandbox_info_t *sandbox_info, bool interactive);

#endif
