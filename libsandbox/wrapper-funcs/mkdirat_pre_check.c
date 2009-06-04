/*
 * mkdir*() pre-check.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

bool sb_mkdirat_pre_check(const char *func, const char *pathname, int dirfd)
{
	char canonic[SB_PATH_MAX];

	save_errno();

	/* XXX: need to check pathname with dirfd */
	if (-1 == canonicalize(pathname, canonic))
		/* see comments in check_syscall() */
		if (ENAMETOOLONG != errno) {
			if (is_env_on(ENV_SANDBOX_DEBUG))
				SB_EINFO("EARLY FAIL", "  %s(%s) @ canonicalize: %s\n",
					func, pathname, strerror(errno));
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
			SB_EINFO("EARLY FAIL", "  %s(%s[%s]) @ lstat: %s\n",
				func, pathname, canonic, strerror(errno));
		errno = EEXIST;
		return false;
	}

	restore_errno();

	return true;
}
