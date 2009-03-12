/*
 * mkdirat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO /* let mkdir() use us */
# define WRAPPER_ARGS_PROTO int dirfd, const char *pathname, mode_t mode
# define WRAPPER_ARGS dirfd, pathname, mode
# define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_AT(dirfd, pathname, 0)
#endif

static inline bool sb_mkdirat_pre_check(WRAPPER_ARGS_PROTO)
{
	char canonic[SB_PATH_MAX];

	save_errno();

	if (-1 == canonicalize(pathname, canonic))
		/* see comments in check_syscall() */
		if (ENAMETOOLONG != errno) {
			if (is_env_on(ENV_SANDBOX_DEBUG))
				SB_EINFO("EARLY FAIL", "  %s(%s) @ canonicalize: %s\n",
					STRING_NAME, pathname, strerror(errno));
			return false;
		}

	/* XXX: Hack to prevent errors if the directory exist, and are
	 * not writable - we rather return EEXIST than fail.  This can
	 * occur if doing something like `mkdir -p /`.  We certainly do
	 * not want to pass this attempt up to the higher levels as those
	 * will trigger a sandbox violation.
	 */
	struct stat st;
	if (0 == lstat(canonic, &st)) {
		if (is_env_on(ENV_SANDBOX_DEBUG))
			SB_EINFO("EARLY FAIL", "  %s(%s) @ lstat: %s\n",
				STRING_NAME, pathname, strerror(errno));
		errno = EEXIST;
		return false;
	}

	restore_errno();

	return true;
}
#define WRAPPER_PRE_CHECKS() sb_mkdirat_pre_check(WRAPPER_ARGS)

#include "__wrapper_simple.c"
