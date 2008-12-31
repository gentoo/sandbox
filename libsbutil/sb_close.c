/*
 * sb_close.c
 *
 * IO functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
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
	if (-1 != res)
		errno = 0;

	return res;
}
