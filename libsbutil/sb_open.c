/*
 * sb_open.c
 *
 * IO functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

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
