/*
 * unlinkat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO /* let unlink() use us */
# define WRAPPER_ARGS_PROTO int dirfd, const char *pathname, int flags
# define WRAPPER_ARGS dirfd, pathname, flags
# define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_AT(dirfd, pathname, flags)
#endif

static inline bool sb_unlinkat_pre_check(WRAPPER_ARGS_PROTO)
{
	char canonic[SB_PATH_MAX];

	save_errno();

	if (-1 == canonicalize(pathname, canonic))
		/* see comments in check_syscall() */
		if (ENAMETOOLONG != errno)
			goto error;

	/* XXX: Hack to make sure sandboxed process cannot remove
	 * a device node, bug #79836. */
	if (0 == strcmp(canonic, "/dev/null") ||
	    0 == strcmp(canonic, "/dev/zero"))
	{
		errno = EACCES;
		goto error;
	}

	restore_errno();

	return true;

 error:
	if (is_env_on(ENV_SANDBOX_DEBUG))
		SB_EINFO("EARLY FAIL", "  %s(%s): %s\n",
			STRING_NAME, pathname, strerror(errno));
	return false;
}
#define WRAPPER_PRE_CHECKS() sb_unlinkat_pre_check(WRAPPER_ARGS)

#include "__wrapper_simple.c"
