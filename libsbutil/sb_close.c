/*
 * sb_close.c
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


/* General purpose function to _reliably_ close a file
 *
 * Returns 0 if successful or negative number on error (and errno set)
 */

int sb_close(int fd)
{
	int res;

	do {
		res = close(fd);
	} while ((res < 0) && (EINTR == errno));

	/* Do not care about errors here */
	errno = 0;

	return res;
}

