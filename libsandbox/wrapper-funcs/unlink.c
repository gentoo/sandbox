/*
 * unlink.c
 *
 * unlink() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

extern int EXTERN_NAME(const char *);
static int (*WRAPPER_TRUE_NAME) (const char *) = NULL;

int WRAPPER_NAME(const char *pathname)
{
	int result = -1, old_errno = errno;
	char canonic[SB_PATH_MAX];

	if (-1 == canonicalize(pathname, canonic))
		/* Path is too long to canonicalize, do not fail, but just let
		 * the real function handle it (see bug #94630 and #21766). */
		if (ENAMETOOLONG != errno)
			return -1;

	/* XXX: Hack to make sure sandboxed process cannot remove
	 * a device node, bug #79836. */
	if ((0 == strncmp(canonic, "/dev/null", 9)) ||
	    (0 == strncmp(canonic, "/dev/zero", 9))) {
		errno = EACCES;
		return result;
	}
	errno = old_errno;

	if FUNCTION_SANDBOX_SAFE("unlink", pathname) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(pathname);
	}

	return result;
}
