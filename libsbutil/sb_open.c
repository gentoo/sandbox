/*
 * sb_open.c
 *
 * IO functions.
 *
 * Copyright 1999-2006 Gentoo Foundation
 *
 *
 *      This program is free software; you can redistribute it and/or modify it
 *      under the terms of the GNU General Public License as published by the
 *      Free Software Foundation version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Header$
 */


#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "sbutil.h"


static int (*sbio_open)(const char *, int, mode_t) = (void *)open;


void sb_set_open(void *new_open)
{
	if (!check_ptr(new_open))
		return;

	sbio_open = new_open;
}

/* General purpose function to _reliably_ open a file
 *
 * Returns the file descriptor or -1 on error (and errno set)
 */

int sb_open(const char *path, int flags, mode_t mode)
{
	int fd;

	do {
		fd = sbio_open(path, flags, mode);
	} while ((-1 == fd) && (EINTR == errno));
	
	if ((-1 != fd) && (0 != errno))
		/* Do not trigger debugging */
		errno = 0;

	if (-1 == fd)
		DBG_MSG("Failed to open file '%s'!\n", path);

	return fd;
}

