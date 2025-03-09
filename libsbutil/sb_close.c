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

/* Quickly close all the open fds (good for daemonization) */
void sb_close_all_fds(void)
{
	DIR *dirp;
	struct dirent *de;
	int dfd, fd;
	const char *fd_dir = sb_get_fd_dir();

	dirp = opendir(fd_dir);
	if (!dirp)
		sb_ebort("could not process %s\n", fd_dir);
	dfd = dirfd(dirp);

	while ((de = readdir(dirp)) != NULL) {
		if (de->d_name[0] == '.')
			continue;
		fd = atoi(de->d_name);
		if (fd != dfd)
			close(fd);
	}

	closedir(dirp);
}
