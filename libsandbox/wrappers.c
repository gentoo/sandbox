/*
 * wrappers.c
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
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 *
 * $Header$
 */


#include "config.h"

/* Better way would be to only define _GNU_SOURCE when __GLIBC__ is defined,
 * but including features.h and then defining _GNU_SOURCE do not work */
#if defined(HAVE_RTLD_NEXT)
# define _GNU_SOURCE
#endif
#include <errno.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include "localdecls.h"
#include "sbutil.h"
#include "libsandbox.h"
#include "wrappers.h"

#if !defined(BROKEN_RTLD_NEXT) && defined(HAVE_RTLD_NEXT)
# define USE_RTLD_NEXT
#endif

/* Macro to check if a wrapper is defined, if not
 * then try to resolve it again. */
#define check_dlsym(_name, _symname, _symver) \
{ \
	int old_errno = errno; \
	if (NULL == _name) \
		_name = get_dlsym(_symname, _symver); \
	errno = old_errno; \
}

static void *libc_handle = NULL;

extern char sandbox_lib[SB_PATH_MAX];
extern int sandbox_on;

/* Need to include the function wrappers here, as they are needed below */
#include "symbols.h"


void *get_dlsym(const char *symname, const char *symver)
{
	void *symaddr = NULL;

	if (NULL == libc_handle) {
#if !defined(USE_RTLD_NEXT)
		libc_handle = dlopen(LIBC_VERSION, RTLD_LAZY);
		if (!libc_handle) {
			fprintf(stderr, "libsandbox:  Can't dlopen libc: %s\n",
				dlerror());
			exit(EXIT_FAILURE);
		}
#else
		libc_handle = RTLD_NEXT;
#endif
	}

	if (NULL == symver)
		symaddr = dlsym(libc_handle, symname);
	else
		symaddr = dlvsym(libc_handle, symname, symver);
	if (!symaddr) {
		fprintf(stderr, "libsandbox:  Can't resolve %s: %s\n",
			symname, dlerror());
		exit(EXIT_FAILURE);
	}

	return symaddr;
}

int libsb_open(const char *pathname, int flags, ...)
{
	va_list ap;
	int mode = 0;
	int result = -1;

	if (flags & O_CREAT) {
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
	}

	check_dlsym(true_open_DEFAULT, symname_open_DEFAULT,
		    symver_open_DEFAULT);
	if (flags & O_CREAT)
		result = true_open_DEFAULT(pathname, flags, mode);
	else
		result = true_open_DEFAULT(pathname, flags);

	return result;
}

char *libsb_getcwd(char *buf, size_t size)
{
	check_dlsym(true_getcwd_DEFAULT, symname_getcwd_DEFAULT,
		    symver_getcwd_DEFAULT);
	
	return true_getcwd_DEFAULT(buf, size);
}

