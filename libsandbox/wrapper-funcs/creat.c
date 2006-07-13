/*
 * creat.c
 *
 * creat() wrapper.
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


extern int EXTERN_NAME(const char *, mode_t);
/* XXX: We use the open() call to simulate create() */
/* static int (*WRAPPER_TRUE_NAME) (const char *, mode_t) = NULL; */

int WRAPPER_NAME(const char *pathname, mode_t mode)
{
	int result = -1;

	if FUNCTION_SANDBOX_SAFE("creat", pathname) {
		check_dlsym(true_open_DEFAULT, symname_open_DEFAULT,
			    symver_open_DEFAULT);
		result = true_open_DEFAULT(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
	}

	return result;
}

