/*
 * wrappers.h
 *
 * Function wrapping functions.
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
 * $Header$
 */

#ifndef __WRAPPERS_H__
#define __WRAPPERS_H__

#include <libsandbox.h>

/* Macro to check if a wrapper is defined, if not
 * then try to resolve it again. */
#define check_dlsym(_name) \
{ \
	int old_errno = errno; \
	if (!true_ ## _name) \
		true_ ## _name = get_dlsym(symname_ ## _name, symver_ ## _name); \
	errno = old_errno; \
}

void *get_dlsym(const char *, const char *);

/* Wrapper for internal use of functions in libsandbox */
int libsb_open(const char *, int, ...);
char *libsb_getcwd(char *, size_t);

#endif /* __WRAPPERS_H__ */

