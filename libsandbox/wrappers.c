/*
 * wrappers.c
 *
 * Function wrapping functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

#include "headers.h"
#include "sbutil.h"
#include "libsandbox.h"
#include "wrappers.h"

#if !defined(BROKEN_RTLD_NEXT) && defined(HAVE_RTLD_NEXT)
# define USE_RTLD_NEXT
#endif

static void *libc_handle = NULL;

static void *get_dlsym(const char *symname, const char *symver)
{
	void *symaddr = NULL;

#if defined(USE_RTLD_NEXT)
	libc_handle = RTLD_NEXT;
#endif

	/* Checking for -1UL is significant on hardened!
	 * USE_RTLD_NEXT returns it as a sign of being unusable.
	 * However using !x or NULL checks does NOT pick it up!
	 * That is also why we need to save/restore errno #260765.
	 */
#define INVALID_LIBC_HANDLE(x) (!x || NULL == x || (void *)-1UL == x)
	if (INVALID_LIBC_HANDLE(libc_handle)) {
		save_errno();
		libc_handle = dlopen(LIBC_VERSION, RTLD_LAZY);
		restore_errno();
		if (INVALID_LIBC_HANDLE(libc_handle)) {
			fprintf(stderr, "libsandbox:  Can't dlopen libc: %s\n",
				dlerror());
			exit(EXIT_FAILURE);
		}
	}
#undef INVALID_LIBC_HANDLE

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

/* Macro to check if a wrapper is defined, if not
 * then try to resolve it again.
 */
#define check_dlsym(_name, _symname, _symver) \
{ \
	if (NULL == _name) \
		_name = get_dlsym(_symname, _symver); \
}

/* Need to include the function wrappers here, as they are needed below */
#include "symbols.h"
