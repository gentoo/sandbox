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

#include <stdio.h>

/* Gentoo style e* printing macro's */
#define EINFO(_args...) \
 do { \
   save_errno (); \
   printf (" \033[32;01m*\033[0m " _args); \
   restore_errno (); \
 } while (0)

#define EWARN(_args...) \
 do { \
   save_errno (); \
   printf (" \033[33;01m*\033[0m " _args); \
   restore_errno (); \
 } while (0)

#define EERROR(_args...) \
 do { \
   save_errno (); \
   fprintf (stderr, " \033[31;01m*\033[0m " _args); \
   restore_errno (); \
 } while (0)

/* String functions.  Return a string on success, or NULL on error
 * or no action taken.  On error errno will be set.*/
char *rc_memrepchr (char **str, char old, char _new, size_t size);
/* Concat two paths adding '/' if needed.  Memory will be allocated
 * with the malloc() call. */
char *rc_strcatpaths (const char *pathname1, const char *pathname2);

/* Compat functions for GNU extensions */
char *rc_strndup (const char *str, size_t size);
/* Same as basename(3), but do not modify path */
char *rc_basename (const char *path);

#endif /* __RC_STRING_H__ */
