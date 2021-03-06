/*
 * vfork() wrapper.
 *
 * Copyright 1999-2021 Gentoo Foundation
 * Licensed under the GPL-2
 */

/* We're only wrapping vfork() as a poor man's pthread_atfork().  That would
 * require dedicated linkage against libpthread.  So here we force the locks
 * to a consistent state before forking.
 *
 * We also implement vfork() as fork() because sandbox does not meet vfork()
 * requirements bet ween vfork()/exec("some-static-bianary") because we launch
 * ptrace in the middle.
 */

#define WRAPPER_ARGS_PROTO
#define WRAPPER_ARGS
#define WRAPPER_SAFE() 0
#define WRAPPER_PRE_CHECKS() \
({ \
	/* pthread_atfork(sb_lock, sb_unlock, sb_unlock); */ \
	sb_lock(); \
	result = sb_unwrapped_fork_DEFAULT(WRAPPER_ARGS_FULL); \
	sb_unlock(); \
	false; \
})
#include "__wrapper_simple.c"
