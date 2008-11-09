/*
 * sb_write.c
 *
 * IO functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
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
