/*
 * fopen64.c
 *
 * fopen64() wrapper.
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


extern FILE *WRAPPER_NAME(const char *, const char *);
static FILE * (*WRAPPER_TRUE_NAME) (const char *, const char *) = NULL;

FILE *WRAPPER_NAME(const char *pathname, const char *mode)
{
	FILE *result = NULL;
	int old_errno = errno;
	struct stat64 st;

	if ((NULL != mode) && (mode[0] == 'r')) {
		/* XXX: If we're trying to read, fail normally if file does
		 *      not stat */
		if (-1 == stat64(pathname, &st)) {
			return NULL;
		}
	}
	errno = old_errno;

	if FUNCTION_SANDBOX_SAFE_OPEN_CHAR("fopen64", pathname, mode) {
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(pathname, mode);
	}

	return result;
}

