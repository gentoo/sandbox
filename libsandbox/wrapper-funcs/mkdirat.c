/*
 * mkdirat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO /* let mkdir() use us */
# define WRAPPER_ARGS_PROTO int dirfd, const char *pathname, mode_t mode
# define WRAPPER_ARGS dirfd, pathname, mode
# define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_AT(dirfd, pathname)
#endif

static inline bool sb_mkdirat_pre_check(WRAPPER_ARGS_PROTO)
{
	char canonic[SB_PATH_MAX];
	save_errno();

	if (-1 == canonicalize(pathname, canonic))
		/* see comments in check_syscall() */
		if (ENAMETOOLONG != errno)
			return false;

	/* XXX: Hack to prevent errors if the directory exist,
	 * and are not writable - we rather return EEXIST rather
	 * than failing */
	struct stat st;
	if (0 == lstat(canonic, &st)) {
		errno = EEXIST;
		return false;
	}

	restore_errno();
	return true;
}
#define WRAPPER_PRE_CHECKS() if (!sb_mkdirat_pre_check(WRAPPER_ARGS)) return result;

#include "__wrapper_simple.c"
