/*
 * unlinkat() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef WRAPPER_ARGS_PROTO /* let unlink() use us */
# define WRAPPER_ARGS_PROTO int dirfd, const char *pathname, int flags
# define WRAPPER_ARGS dirfd, pathname, flags
# define WRAPPER_SAFE() FUNCTION_SANDBOX_SAFE_AT(dirfd, pathname)
#endif

static inline bool sb_unlinkat_pre_check(WRAPPER_ARGS_PROTO)
{
	char canonic[SB_PATH_MAX];
	save_errno();

	if (-1 == canonicalize(pathname, canonic))
		/* Path is too long to canonicalize, do not fail, but just let
		 * the real function handle it (see bug #94630 and #21766). */
		if (ENAMETOOLONG != errno)
			return false;

	/* XXX: Hack to make sure sandboxed process cannot remove
	 * a device node, bug #79836. */
	if (0 == strcmp(canonic, "/dev/null") ||
	    0 == strcmp(canonic, "/dev/zero"))
	{
		errno = EACCES;
		return false;
	}

	restore_errno();
	return true;
}
#define WRAPPER_PRE_CHECKS() if (!sb_unlinkat_pre_check(WRAPPER_ARGS)) return result;

#include "__wrapper_simple.c"
