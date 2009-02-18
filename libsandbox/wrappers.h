/*
 * wrappers.h
 *
 * Function wrapping functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef __WRAPPERS_H__
#define __WRAPPERS_H__

#include <libsandbox.h>

void *get_dlsym(const char *, const char *);

/* Wrapper for internal use of functions in libsandbox */
int libsb_open(const char *, int, ...);

#define                sb_unwrapped_getcwd sb_unwrapped_getcwd_DEFAULT
attribute_hidden char *sb_unwrapped_getcwd (char *, size_t);

#endif /* __WRAPPERS_H__ */
