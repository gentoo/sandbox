/*
 * string.h
 *
 * Miscellaneous string related macro's and functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
 */

#ifndef __RC_STRING_H__
#define __RC_STRING_H__

/* Concat two paths adding '/' if needed.  Memory will be allocated
 * with the malloc() call. */
char *rc_strcatpaths (const char *pathname1, const char *pathname2);

#endif /* __RC_STRING_H__ */
