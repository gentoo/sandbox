/*
 * fork() wrapper.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

/* We're only wrapping fork() as a poor man's pthread_atfork().  That would
 * require dedicated linkage against libpthread.  So here we force the locks
 * to a consistent state before forking. #263657
 */

#define WRAPPER_ARGS_PROTO
#define WRAPPER_ARGS
#define WRAPPER_SAFE() 0
#define WRAPPER_PRE_CHECKS() \
({ \
	/* pthread_atfork(sb_lock, sb_unlock, sb_unlock); */ \
	sb_lock(); \
	result = SB_HIDDEN_FUNC(WRAPPER_NAME)(WRAPPER_ARGS_FULL); \
	sb_unlock(); \
	false; \
})
#include "__wrapper_simple.c"
