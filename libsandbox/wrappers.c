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

#undef _FILE_OFFSET_BITS
#undef _TIME_BITS
#define _LARGEFILE64_SOURCE

#include "headers.h"
#include "sbutil.h"
#include "libsandbox.h"
#include "wrappers.h"

#if !defined(BROKEN_RTLD_NEXT) && defined(HAVE_RTLD_NEXT)
# define USE_RTLD_NEXT
#endif

static void *libc_handle(void)
{
	static void *handle;
	if (!handle) {
		save_errno();	/* #260765 */
		handle = dlopen(LIBC_VERSION, RTLD_LAZY);
		if (!handle)
			fprintf(stderr, "libsandbox:  Can't dlopen libc: %s\n",
				dlerror());
		restore_errno();
	}
	return handle;
}

static void *get_symbol(void *handle, const char *symbol, const char *version)
{
	void *symaddr;

	if (!version)
		symaddr = dlsym(handle, symbol);
	else
		symaddr = dlvsym(handle, symbol, version);

	if (!symaddr) {
		fprintf(stderr, "libsandbox:  Can't resolve %s: %s\n",
			symbol, dlerror());
		exit(EXIT_FAILURE);
	}

	return symaddr;
}

void *sb_get_symbol(const char *symbol, const char *version)
{
#ifdef USE_RTLD_NEXT
	void *handle = RTLD_NEXT;
#else
	void *handle = libc_handle();
#endif
	return get_symbol(handle, symbol, version);
}

void *sb_libc_symbol(const char *symbol)
{
	void *handle = libc_handle();
	if (!handle)
#ifdef USE_RTLD_NEXT
		/* Use RTLD_NEXT as a fallback if libc cannot be found, bug 973482 */
		handle = RTLD_NEXT;
#else
		exit(EXIT_FAILURE);
#endif
	return get_symbol(handle, symbol, NULL);
}

/* Macro to check if a wrapper is defined, if not
 * then try to resolve it again.
 */
#define check_dlsym(_name, _symname, _symver) \
{ \
	if (NULL == _name) \
		_name = sb_get_symbol(_symname, _symver); \
}

/* Need to include the function wrappers here, as they are needed below */
#include "symbols.h"
