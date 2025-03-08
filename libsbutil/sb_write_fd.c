/*
 * helper for sucking up a file and writing it to a fd.
 * good for copying the contents of small status files
 * into a log file.
 *
 * Copyright 1999-2012 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

int sb_copy_file_to_fd(const char *file, int ofd)
{
	char buf[128];
	int ret = -1;

	int ifd = sb_open(file, O_RDONLY|O_CLOEXEC, 0);
	if (ifd == -1)
		return ret;

	while (1) {
		size_t len = sb_read(ifd, buf, sizeof(buf));
		if (len == -1)
			goto error;
		else if (!len)
			break;
		size_t i;
		for (i = 0; i < len; ++i)
			if (!buf[i])
				buf[i] = ' ';
		if (sb_write(ofd, buf, len) != len)
			goto error;
	}

	ret = 0;
 error:
	sb_close(ifd);
	return ret;
}
