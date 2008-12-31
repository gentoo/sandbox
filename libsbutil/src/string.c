/*
 * string.c
 *
 * Miscellaneous macro's and functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "rcscripts/rcutil.h"

char *
rc_strcatpaths (const char *pathname1, const char *pathname2)
{
  char *new_path = NULL;
  int lenght;

  if ((!check_arg_str (pathname1)) || (!check_arg_str (pathname2)))
    return 0;

  /* Lenght of pathname1 + lenght of pathname2 + '/' if needed */
  lenght = strlen (pathname1) + strlen (pathname2) + 2;
  /* lenght + '\0' */
  new_path = xmalloc (lenght);

  snprintf (new_path, lenght, "%s%s%s", pathname1,
	    (pathname1[strlen (pathname1) - 1] != '/') ? "/" : "",
	    pathname2);

  return new_path;
}
