/*
 * unlink*() pre-check.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

bool sb_unlinkat_pre_check(const char *func, const char *pathname, int dirfd)
{
	char canonic[SB_PATH_MAX];
	char dirfd_path[SB_PATH_MAX];

	save_errno();

	/* Expand the dirfd path first */
	switch (resolve_dirfd_path(dirfd, pathname, dirfd_path, sizeof(dirfd_path))) {
		case -1:
			if (is_env_on(ENV_SANDBOX_DEBUG))
				SB_EINFO("EARLY FAIL", "  %s(%s) @ resolve_dirfd_path: %s\n",
					func, pathname, strerror(errno));
			return false;
		case 0:
			pathname = dirfd_path;
			break;
	}

	/* Then break down any relative/symlink paths */
	if (-1 == canonicalize(pathname, canonic))
		/* see comments in check_syscall() */
		if (ENAMETOOLONG != errno) {
			if (is_env_on(ENV_SANDBOX_DEBUG))
				SB_EINFO("EARLY FAIL", "  %s(%s) @ canonicalize: %s\n",
					func, pathname, strerror(errno));
			return false;
		}

	/* XXX: Hack to make sure sandboxed process cannot remove
	 * a device node, bug #79836. */
	if (0 == strcmp(canonic, "/dev/null") ||
	    0 == strcmp(canonic, "/dev/zero"))
	{
		errno = EACCES;
		if (is_env_on(ENV_SANDBOX_DEBUG))
			SB_EINFO("EARLY FAIL", "  %s(%s): %s\n",
				func, pathname, strerror(errno));
		return false;
	}

	restore_errno();

	return true;
}
