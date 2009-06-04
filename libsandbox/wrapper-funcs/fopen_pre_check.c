/*
 * fopen() pre-check.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

bool sb_fopen_pre_check(const char *func, const char *pathname, const char *mode)
{
	if ((NULL != mode) && (mode[0] == 'r')) {
		save_errno();

		/* If we're trying to read, fail normally if file does not stat */
		struct stat st;
		if (-1 == stat(pathname, &st)) {
			if (is_env_on(ENV_SANDBOX_DEBUG))
				SB_EINFO("EARLY FAIL", "  %s(%s): %s\n",
					func, pathname, strerror(errno));
			return false;
		}

		restore_errno();
	}

	return true;
}
