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

/* Macros to check if a function should be executed */
#define FUNCTION_SANDBOX_SAFE(_func, _path) \
	((0 == is_sandbox_on()) || (1 == before_syscall(_func, _path)))

#define FUNCTION_SANDBOX_SAFE_ACCESS(_func, _path, _flags) \
	((0 == is_sandbox_on()) || (1 == before_syscall_access(_func, _path, _flags)))

#define FUNCTION_SANDBOX_SAFE_OPEN_INT(_func, _path, _flags) \
	((0 == is_sandbox_on()) || (1 == before_syscall_open_int(_func, _path, _flags)))

#define FUNCTION_SANDBOX_SAFE_OPEN_CHAR(_func, _path, _mode) \
	((0 == is_sandbox_on()) || (1 == before_syscall_open_char(_func, _path, _mode)))

void *get_dlsym(const char *, const char *);

/* Wrapper for internal use of functions in libsandbox */
int libsb_open(const char *, int, ...);
char *libsb_getcwd(char *, size_t);

#endif /* __WRAPPERS_H__ */

