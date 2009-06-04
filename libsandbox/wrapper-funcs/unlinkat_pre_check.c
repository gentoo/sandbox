/*
 * unlink*() pre-check.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

bool sb_unlinkat_pre_check(const char *func, const char *pathname, int dirfd)
{
	char canonic[SB_PATH_MAX];

	save_errno();

	/* XXX: need to check pathname with dirfd */
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
			func, pathname, strerror(errno));
	return false;
}
