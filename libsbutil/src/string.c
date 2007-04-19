/*
 * string.c
 *
 * Miscellaneous macro's and functions.
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

#include <string.h>
#include <stdlib.h>

#include "rcscripts/rcutil.h"

char *
rc_memrepchr (char **str, char old, char new, size_t size)
{
  char *str_p;

  if (!check_arg_strv (str))
    return NULL;

  str_p = memchr (*str, old, size);

  while (NULL != str_p)
    {
      str_p[0] = new;
      str_p = memchr (&str_p[1], old, size - (str_p - *str) - 1);
    }

  return *str;
}

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

