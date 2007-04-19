/*
 * debug.c
 *
 * Simle debugging/logging macro's and functions.
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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "rcscripts/rcutil.h"

volatile static bool debug_enabled = TRUE;
volatile static int debug_errno = 0;

static char log_domain_default[] = "rcscripts";
volatile static char *log_domain = log_domain_default;

void
rc_log_domain (const char *new_domain)
{
  if (check_str (new_domain))
    log_domain = (char *)new_domain;
}

void
rc_debug_enabled (bool enabled)
{
  debug_enabled = enabled;
}

void
rc_errno_set (int rc_errno)
{
  if (rc_errno >= 0)
    debug_errno = rc_errno;
}

void
rc_errno_clear (void)
{
  debug_errno = 0;
}

int
rc_errno_get (void)
{
  return debug_errno;
}

bool
rc_errno_is_set (void)
{
  return (debug_errno > 0);
}

void
debug_message (const char *file, const char *func, int line,
	       const char *format, ...)
{
  va_list arg;
  char *format_str;
  int length;

#if !defined(RC_DEBUG)
  if (!debug_enabled)
    return;
#endif

  length = strlen (log_domain) + strlen ("():       ") + 1;
  /* Do not use xmalloc() here, else we may have recursive issues */
  format_str = malloc (length);
  if (NULL == format_str)
    {
      fprintf (stderr, "(%s) error: in %s, function %s(), line %i:\n",
	       log_domain, __FILE__, __FUNCTION__, __LINE__);
      fprintf (stderr, "(%s)        Failed to allocate buffer!\n",
	       log_domain);
      abort ();
    }

  snprintf (format_str, length, "(%s)      ", log_domain);

  va_start (arg, format);

#if !defined(RC_DEBUG)
  /* Bit of a hack, as how we do things tend to cause seek
   * errors when reading the parent/child pipes */
  /* if ((0 != errno) && (ESPIPE != errno)) { */
  if (rc_errno_is_set ())
    {
#endif
      if (rc_errno_is_set ())
	fprintf (stderr, "(%s) error: ", log_domain);
      else
	fprintf (stderr, "(%s) debug: ", log_domain);

      fprintf (stderr, "in %s, function %s(), line %i:\n", file, func, line);

      if (rc_errno_is_set ())
	fprintf (stderr, "%s  strerror() = '%s'\n", format_str, strerror (rc_errno_get ()));

      fprintf (stderr, "%s  ", format_str);
      vfprintf (stderr, format, arg);
#if !defined(RC_DEBUG)
    }
#endif

  va_end (arg);

  free (format_str);
}

inline bool
check_ptr (const void *ptr)
{
  if (NULL == ptr)
    return FALSE;

  return TRUE;
}

inline bool
check_str (const char *str)
{
  if ((NULL == str) || (0 == strlen (str)))
    return FALSE;

  return TRUE;
}

inline bool
check_strv (char **str)
{
  if ((NULL == str) || (NULL == *str) || (0 == strlen (*str)))
    return FALSE;

  return TRUE;
}

inline bool
check_fd (int fd)
{
  if ((0 >= fd) || (-1 == fcntl (fd, F_GETFL)))
    return FALSE;

  return TRUE;
}

inline bool
check_fp (FILE *fp)
{
  if ((NULL == fp) || (-1 == fileno (fp)))
    return FALSE;

  return TRUE;
}

inline bool
__check_arg_ptr (const void *ptr, const char *file, const char *func, size_t line)
{
  if (!check_ptr (ptr))
    {
      rc_errno_set (EINVAL);

      debug_message (file, func, line, "Invalid pointer passed!\n");

      return FALSE;
    }

  return TRUE;
}

inline bool
__check_arg_str (const char *str, const char *file, const char *func, size_t line)
{
  if (!check_str (str))
    {
      rc_errno_set (EINVAL);

      debug_message (file, func, line, "Invalid string passed!\n");

      return FALSE;
    }

  return TRUE;
}

inline bool
__check_arg_strv (char **str, const char *file, const char *func, size_t line)
{
  if (!check_strv (str))
    {
      rc_errno_set (EINVAL);

      debug_message (file, func, line, "Invalid string array passed!\n");

      return FALSE;
    }

  return TRUE;
}

inline bool
__check_arg_fd (int fd, const char *file, const char *func, size_t line)
{
  if (!check_fd (fd))
    {
      rc_errno_set (EBADF);

      debug_message (file, func, line, "Invalid file descriptor passed!\n");

      return FALSE;
    }

  return TRUE;
}

inline bool
__check_arg_fp (FILE *fp, const char *file, const char *func, size_t line)
{
  if (!check_fp (fp))
    {
      rc_errno_set (EBADF);

      debug_message (file, func, line, "Invalid file descriptor passed!\n");

      return FALSE;
    }

  return TRUE;
}

inline void *
__xcalloc(size_t nmemb, size_t size, const char *file,
	  const char *func, size_t line)
{
  void *new_ptr;

  new_ptr = calloc (nmemb, size);
  if (NULL == new_ptr)
    {
      /* Set errno in case specific malloc() implementation does not */
      rc_errno_set (ENOMEM);

      debug_message (file, func, line, "Failed to allocate buffer!\n");

      return NULL;
    }

  return new_ptr;
}

inline void *
__xmalloc (size_t size, const char *file, const char *func, size_t line)
{
  void *new_ptr;

  new_ptr = malloc (size);
  if (NULL == new_ptr)
    {
      /* Set errno in case specific malloc() implementation does not */
      rc_errno_set (ENOMEM);

      debug_message (file, func, line, "Failed to allocate buffer!\n");

      return NULL;
    }

  return new_ptr;
}

inline void *
__xrealloc (void *ptr, size_t size, const char *file,
	    const char *func, size_t line)
{
  void *new_ptr;

  new_ptr = realloc (ptr, size);
  if (NULL == new_ptr)
    {
      /* Set errno in case specific realloc() implementation does not */
      rc_errno_set (ENOMEM);

      debug_message (file, func, line, "Failed to reallocate buffer!\n");

      return NULL;
    }

  return new_ptr;
}

inline char *
__xstrndup (const char *str, size_t size, const char *file,
	    const char *func, size_t line)
{
  char *new_ptr;

  new_ptr = rc_strndup (str, size);
  if (NULL == new_ptr)
    {
      /* Set errno in case specific realloc() implementation does not */
      rc_errno_set (ENOMEM);

      debug_message (file, func, line,
		     "Failed to duplicate string via rc_strndup() !\n");

      return NULL;
    }

  return new_ptr;
}

