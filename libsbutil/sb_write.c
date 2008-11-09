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

/* General purpose function to _reliably_ write to a file
 *
 * If returned value is less than count, there was a fatal
 * error and value tells how many bytes were actually written
 */

size_t sb_write(int fd, const void *buf, size_t count)
{
	ssize_t n;
	size_t accum = 0;

	do {
		n = write(fd, buf + accum, count - accum);
		if (n < 0) {
			if (EINTR == errno) {
				/* Reset errno to not trigger DBG_MSG */
				errno = 0;
				continue;
			}

			DBG_MSG("Failed to write to fd=%i!\n", fd);
			break;
		}

		accum += n;
	} while (accum < count);

	return accum;
}
