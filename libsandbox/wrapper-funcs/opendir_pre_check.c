/*
 * opendir() pre-check.
 *
 * Copyright 1999-2016 Gentoo Foundation
 * Licensed under the GPL-2
 */

bool sb_opendir_pre_check(const char *func, const char *name)
{
	/* If length of name is larger than PATH_MAX, we would mess it up
	 * before it reaches the open syscall, which would cleanly error out
	 * via sandbox as well (actually with much smaller lengths than even
	 * PATH_MAX).
	 * So error out early in this case, in order to avoid an abort in
	 * check_syscall later on, which gets ran for opendir, despite it not
	 * being a syscall.
	 */
	if (strnlen(name, PATH_MAX) == PATH_MAX) {
		errno = ENAMETOOLONG;
		sb_debug_dyn("EARLY FAIL: %s(%s): %s\n",
			func, name, strerror(errno));
		return false;
	}

	return true;
}
