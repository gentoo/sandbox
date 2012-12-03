/*
 * sb_gdb.c
 *
 * Helpers for autolaunching gdb.
 *
 * Copyright 1999-2012 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

static int __sb_connect_gdb(void *vpid)
{
	char pid[10];
	snprintf(pid, sizeof(pid), "%i", (pid_t)(uintptr_t)vpid);
	unsetenv(ENV_LD_PRELOAD);
	return /*sb_unwrapped_*/execlp("gdb", "gdb",
		"--quiet",
		"--pid", pid,
		"-ex", "set _gdb_poller = 0",
		"-ex", "fin",
		"-ex", "bt full",
		"-ex", "echo \\n",
		"-ex", "f",
		NULL);
}

#define STACK_SIZE 4096

#ifndef HAVE_CLONE
# ifdef HAVE___CLONE2
/* Hrm, maybe they have clone2 */
#  define clone(fn, stack, flags, arg) \
	__clone2(fn, stack, STACK_SIZE, flags, arg, NULL, NULL, NULL)
# else
/* Fake it with fork() ... not as safe, but not much else we can do */
static int fake_clone(int (*fn)(void *), void *child_stack, int flags, void *arg)
{
	pid_t pid = fork();
	switch (pid) {
	case 0:
		_exit(fn(arg));
	default:
		return pid;
	}
}
#  define clone(...) fake_clone(__VA_ARGS__)
# endif
#endif

void sb_gdb(void)
{
	char stack[STACK_SIZE * 2];
	pid_t pid = getpid();
	/* Put the child stack in the middle so we don't have to worry about
	 * the direction of growth.  Most grown down, but some grow up!@
	 */
	if (clone(__sb_connect_gdb, stack + STACK_SIZE, 0, (void *)(uintptr_t)pid) != -1) {
		volatile int _gdb_poller = 1;
		while (_gdb_poller);
	}
}

void sb_maybe_gdb(void)
{
	if (is_env_on("SANDBOX_GDB")) {
		sb_einfo("attempting to autolaunch gdb; please wait ...\n\n");
		sb_gdb();
	}
}
