/*
 * open64.c
 *
 * open64() wrapper.
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


extern int EXTERN_NAME(const char *, int, ...);
static int (*WRAPPER_TRUE_NAME) (const char *, int, ...) = NULL;

/* Eventually, there is a third parameter: it's mode_t mode */
int WRAPPER_NAME(const char *pathname, int flags, ...)
{
	va_list ap;
	int mode = 0;
	int result = -1;
	int old_errno = errno;
	struct stat64 st;

	if (flags & O_CREAT) {
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
	} else {
		/* XXX: If we're not trying to create, fail normally if
		 *      file does not stat */
		if (-1 == stat64(pathname, &st)) {
			return -1;
		}
	}
	errno = old_errno;

	if FUNCTION_SANDBOX_SAFE_OPEN_INT("open64", pathname, flags) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		if (flags & O_CREAT)
			result = WRAPPER_TRUE_NAME(pathname, flags, mode);
		else
			result = WRAPPER_TRUE_NAME(pathname, flags);
	}

	return result;
}

