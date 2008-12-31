/*
 * debug.c
 *
 * Simle debugging/logging macro's and functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "rcscripts/rcutil.h"

volatile static int debug_errno = 0;

static char log_domain_default[] = "rcscripts";
static char *log_domain = log_domain_default;

void
rc_log_domain (const char *new_domain)
{
  if (check_str (new_domain))
    log_domain = (char *)new_domain;
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

  length = strlen (log_domain) + strlen ("():       ") + 1;
  format_str = xmalloc (length);
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

bool
check_ptr (const void *ptr)
{
  if (NULL == ptr)
    return false;

  return true;
}

bool
check_str (const char *str)
{
  if ((NULL == str) || (0 == strlen (str)))
    return false;

  return true;
}

bool
check_strv (char **str)
{
  if ((NULL == str) || (NULL == *str) || (0 == strlen (*str)))
    return false;

  return true;
}

bool
check_fd (int fd)
{
  if ((0 >= fd) || (-1 == fcntl (fd, F_GETFL)))
    return false;

  return true;
}

bool
check_fp (FILE *fp)
{
  if ((NULL == fp) || (-1 == fileno (fp)))
    return false;

  return true;
}

bool
__check_arg_ptr (const void *ptr, const char *file, const char *func, size_t line)
{
  if (!check_ptr (ptr))
    {
      rc_errno_set (EINVAL);

      debug_message (file, func, line, "Invalid pointer passed!\n");

      return false;
    }

  return true;
}

bool
__check_arg_str (const char *str, const char *file, const char *func, size_t line)
{
  if (!check_str (str))
    {
      rc_errno_set (EINVAL);

      debug_message (file, func, line, "Invalid string passed!\n");

      return false;
    }

  return true;
}

bool
__check_arg_strv (char **str, const char *file, const char *func, size_t line)
{
  if (!check_strv (str))
    {
      rc_errno_set (EINVAL);

      debug_message (file, func, line, "Invalid string array passed!\n");

      return false;
    }

  return true;
}

bool
__check_arg_fd (int fd, const char *file, const char *func, size_t line)
{
  if (!check_fd (fd))
    {
      rc_errno_set (EBADF);

      debug_message (file, func, line, "Invalid file descriptor passed!\n");

      return false;
    }

  return true;
}

bool
__check_arg_fp (FILE *fp, const char *file, const char *func, size_t line)
{
  if (!check_fp (fp))
    {
      rc_errno_set (EBADF);

      debug_message (file, func, line, "Invalid file descriptor passed!\n");

      return false;
    }

  return true;
}
