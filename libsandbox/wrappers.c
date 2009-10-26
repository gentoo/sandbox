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

static void *libc_handle;

static void load_libc_handle(void)
{
	save_errno();	/* #260765 */
	libc_handle = dlopen(LIBC_VERSION, RTLD_LAZY);
	restore_errno();

	if (!libc_handle) {
		fprintf(stderr, "libsandbox:  Can't dlopen libc: %s\n",
			dlerror());
		exit(EXIT_FAILURE);
	}
}

static void *get_dlsym(const char *symname, const char *symver)
{
	void *symaddr;

	if (!libc_handle) {
#ifdef USE_RTLD_NEXT
		libc_handle = RTLD_NEXT;
 try_again: ;
#else
		load_libc_handle();
#endif
	}

	if (NULL == symver)
		symaddr = dlsym(libc_handle, symname);
	else
		symaddr = dlvsym(libc_handle, symname, symver);

	if (!symaddr) {
#ifdef USE_RTLD_NEXT
		/* Maybe RTLD_NEXT is broken for some screwed up reason as
		 * can be seen with some specific glibc/kernel versions.
		 * Recover dynamically so that we can be deployed easily
		 * via binpkgs and upgrades #202765 #206678
		 */
		if (libc_handle == RTLD_NEXT) {
			load_libc_handle();
			goto try_again;
		}
#endif

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
