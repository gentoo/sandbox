/*
 * Copyright 2025 Gentoo Authors
 * Distributed under the terms of the GNU General Public License v2
 */

#include "headers.h"
#include "sbutil.h"

__attribute__ ((weak))
char *sb_getcwd(char *buf, size_t size)
{
	return getcwd(buf, size);
}
