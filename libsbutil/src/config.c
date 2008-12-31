/*
 * misc.c
 *
 * Miscellaneous macro's and functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "rcscripts/rcutil.h"

/* This handles simple 'entry="bar"' type variables.  If it is more complex
 * ('entry="$(pwd)"' or such), it will obviously not work, but current behaviour
 * should be fine for the type of variables we want. */
char *
rc_get_cnf_entry (const char *pathname, const char *entry, const char *sep)
{
  rc_dynbuf_t *dynbuf = NULL;
  char *buf = NULL;
  char *str_ptr;
  char *value = NULL;
  char *token;

  rc_errno_save ();

  if ((!check_arg_str (pathname)) || (!check_arg_str (entry)))
    return NULL;

  /* If it is not a file or symlink pointing to a file, bail */
  if (!rc_is_file (pathname, true))
    {
      rc_errno_set (ENOENT);
      DBG_MSG ("'%s' is not a file or does not exist!\n", pathname);
      return NULL;
    }

  if (0 == rc_get_size (pathname, true))
    {
      /* XXX: Should we set errno here ? */
      DBG_MSG ("'%s' have a size of 0!\n", pathname);
      return NULL;
    }

  dynbuf = rc_dynbuf_new_mmap_file (pathname);
  if (NULL == dynbuf)
    {
      DBG_MSG ("Could not open config file for reading!\n");
      return NULL;
    }

  /* Make sure we do not get false positives below */
  rc_errno_clear ();
  while (NULL != (buf = rc_dynbuf_read_line (dynbuf)))
    {
      str_ptr = buf;

      /* Strip leading spaces/tabs */
      while ((str_ptr[0] == ' ') || (str_ptr[0] == '\t'))
	str_ptr++;

      /* Get entry and value */
      token = strsep (&str_ptr, "=");
      /* Bogus entry or value */
      if (NULL == token)
	goto _continue;

      /* Make sure we have a string that is larger than 'entry', and
       * the first part equals 'entry' */
      if ((strlen (token) > 0) && (0 == strcmp (entry, token)))
	{
	  do
	    {
	      /* Bash variables are usually quoted */
	      token = strsep (&str_ptr, "\"\'");
	      /* If quoted, the first match will be "" */
	    }
	  while ((NULL != token) && (0 == strlen (token)));

	  /* We have a 'entry='.  We respect bash rules, so NULL
	   * value for now (if not already) */
	  if (NULL == token)
	    {
	      /* We might have 'entry=' and later 'entry="bar"',
	       * so just continue for now ... we will handle
	       * it below when 'value == NULL' */
	      if ((!check_str(sep)) && (NULL != value))
		{
		  free (value);
		  value = NULL;
		}
	      goto _continue;
	    }

	  if ((!check_str(sep)) ||
	      ((check_str(sep)) && (NULL == value)))
	    {
	      /* If we have already allocated 'value', free it */
	      if (NULL != value)
		free (value);

	      value = xstrdup (token);
	    }
	  else
	    {
	      value = xrealloc (value, strlen(value) + strlen(token) +
				strlen(sep) + 1);
	      snprintf(value + strlen(value), strlen(token) + strlen(sep) + 1,
		       "%s%s", sep, token);
	    }

	  /* We do not break, as there might be more than one entry
	   * defined, and as bash uses the last, so should we */
	  /* break; */
	}

_continue:
      free (buf);
    }

  /* rc_dynbuf_read_line() returned NULL with errno set */
  if ((NULL == buf) && (rc_errno_is_set ()))
    {
      DBG_MSG ("Failed to read line from dynamic buffer!\n");
      rc_dynbuf_free (dynbuf);
      if (NULL != value)
	free (value);

      return NULL;
    }


  if (NULL == value)
    /* NULL without errno set means everything went OK, but we did not get a
     * entry */
    DBG_MSG ("Failed to get value for config entry '%s'!\n", entry);

  rc_dynbuf_free (dynbuf);

  rc_errno_restore ();

  return value;
}
