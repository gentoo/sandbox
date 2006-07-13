/*
 * unlink.c
 *
 * unlink() wrapper.
 *
 * Copyright 1999-2006 Gentoo Foundation
 *
 *
 *      This program is free software; you can redistribute it and/or modify it
 *      under the terms of the GNU General Public License as published by the
 *      Free Software Foundation version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 *
 * $Header$
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

