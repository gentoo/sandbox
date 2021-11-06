/*
 * sb_close.c
 *
 * IO functions.
 *
 * Copyright 1999-2012 Gentoo Foundation
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

/* Quickly close all the open fds (good for daemonization) */
void sb_close_all_fds(void)
{
	DIR *dirp;
	struct dirent64 *de;
	int dfd, fd;
	const char *fd_dir = sb_get_fd_dir();

	dirp = opendir(fd_dir);
	if (!dirp)
		sb_ebort("could not process %s\n", fd_dir);
	dfd = dirfd(dirp);

	while ((de = readdir64(dirp)) != NULL) {
		if (de->d_name[0] == '.')
			continue;
		fd = atoi(de->d_name);
		if (fd != dfd)
			close(fd);
	}

	closedir(dirp);
}
