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

/* Wrapper for internal use of functions in libsandbox.  Should really
 * autogenerate this header ...
 */
#define                sb_unwrapped_access sb_unwrapped_access_DEFAULT
attribute_hidden int   sb_unwrapped_access (const char *, int);
#define                sb_unwrapped_getcwd sb_unwrapped_getcwd_DEFAULT
attribute_hidden char *sb_unwrapped_getcwd (char *, size_t);
#define                sb_unwrapped_open   sb_unwrapped_open_DEFAULT
attribute_hidden int   sb_unwrapped_open   (const char *, int, mode_t);

#endif
