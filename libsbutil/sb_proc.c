/*
 * funcs for poking around /proc
 *
 * Copyright 1999-2012 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

const char sb_fd_dir[] =
#if defined(SANDBOX_PROC_SELF_FD)
	"/proc/self/fd"
#elif defined(SANDBOX_DEV_FD)
	"/dev/fd"
#else
# error "how do i access a proc's fd/ tree ?"
#endif
;

const char *sb_get_cmdline(pid_t pid)
{
#if !defined(SANDBOX_PROC_1_CMDLINE) && !defined(SANDBOX_PROC_SELF_CMDLINE) && !defined(SANDBOX_PROC_dd_CMDLINE)
# error "how do i access a proc's cmdline ?"
#endif
	static char path[256];
	if (!pid)
		pid = getpid();
	sprintf(path, "/proc/%i/cmdline", pid);
	return path;
}
