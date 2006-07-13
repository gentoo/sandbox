/*
 * sb_write.c
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


/* General purpose function to _reliably_ read from a file.
 *
 * Returns total read bytes or -1 on error.
 */

size_t sb_read(int fd, void *buf, size_t count)
{
	ssize_t n;
	size_t accum = 0;

	do {
		n = read(fd, buf + accum, count - accum);

		if (n > 0) {
			accum += n;
			continue;
		}

		if (n < 0) {
			if (EINTR == errno) {
				/* Reset errno to not trigger DBG_MSG */
				errno = 0;
				continue;
			}

			DBG_MSG("Failed to read from fd=%i!\n", fd);
			return -1;
		}

		/* Found EOF */
		break;
	} while (accum < count);

	return accum;
}

