/*
 * string.h
 *
 * Miscellaneous string related macro's and functions.
 *
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
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
