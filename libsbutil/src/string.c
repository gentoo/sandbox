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
  if (NULL == new_path)
    return NULL;

  snprintf (new_path, lenght, "%s%s%s", pathname1,
	    (pathname1[strlen (pathname1) - 1] != '/') ? "/" : "",
	    pathname2);

  return new_path;
}

char *
rc_strndup (const char *str, size_t size)
{
  char *new_str = NULL;
  size_t len;

  /* We cannot check if its a valid string here, as it might
   * not be '\0' terminated ... */
  if (!check_arg_ptr (str))
    return NULL;

  /* Check lenght of str without breaching the size limit */
  for (len = 0; (len < size) && ('\0' != str[len]); len++);

  new_str = xmalloc (len + 1);
  if (NULL == new_str)
    return NULL;

  /* Make sure our string is NULL terminated */
  new_str[len] = '\0';

  return (char *) memcpy (new_str, str, len);
}

char *
rc_basename (const char *path)
{
  char *new_path = NULL;

  if (!check_arg_str (path))
    return NULL;

  /* Copied from glibc */
  new_path = strrchr (path, '/');
  return new_path ? new_path + 1 : (char *) path;
}
