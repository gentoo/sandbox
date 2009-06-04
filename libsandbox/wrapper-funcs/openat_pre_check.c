/*
 * open*() pre-check.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

bool sb_openat_pre_check(const char *func, const char *pathname, int dirfd, int flags)
{
	if (!(flags & O_CREAT)) {
		/* If we're not trying to create, fail normally if
		 * file does not stat
		 */
		if (dirfd == AT_FDCWD || pathname[0] == '/') {
			struct stat st;
			save_errno();
			if (-1 == stat(pathname, &st)) {
				if (is_env_on(ENV_SANDBOX_DEBUG))
					SB_EINFO("EARLY FAIL", "  %s(%s): %s\n",
						func, pathname, strerror(errno));
				return false;
			}
			restore_errno();
		}
	}

	return true;
}
