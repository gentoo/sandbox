/*
 * dynbuf.c
 *
 * Dynamic allocated buffers.
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rcscripts/rcutil.h"

#define DYNAMIC_BUFFER_SIZE (sizeof (char) * 2 * 1024)

static rc_dynbuf_t *rc_dynbuf_reallocate (rc_dynbuf_t *dynbuf, size_t needed);

rc_dynbuf_t *
rc_dynbuf_new (void)
{
  rc_dynbuf_t *dynbuf = NULL;

  dynbuf = xmalloc (sizeof (rc_dynbuf_t));
  if (NULL == dynbuf)
    return NULL;

  dynbuf->data = xmalloc (DYNAMIC_BUFFER_SIZE);
  if (NULL == dynbuf->data)
    {
      free (dynbuf);
      return NULL;
    }

  dynbuf->length = DYNAMIC_BUFFER_SIZE;
  dynbuf->rd_index = 0;
  dynbuf->wr_index = 0;
  dynbuf->file_map = FALSE;

  return dynbuf;
}

rc_dynbuf_t *
rc_dynbuf_new_mmap_file (const char *name)
{
  rc_dynbuf_t *dynbuf = NULL;

  dynbuf = xmalloc (sizeof (rc_dynbuf_t));
  if (NULL == dynbuf)
    return NULL;

  if (-1 == rc_file_map (name, &dynbuf->data, &dynbuf->length))
    {
      DBG_MSG ("Failed to mmap file '%s'\n", name);
      free (dynbuf);

      return NULL;
    }

  dynbuf->wr_index = dynbuf->length;
  dynbuf->rd_index = 0;
  dynbuf->file_map = TRUE;

  return dynbuf;
}

rc_dynbuf_t *
rc_dynbuf_reallocate (rc_dynbuf_t *dynbuf, size_t needed)
{
  int len;

  if (!rc_check_arg_dynbuf (dynbuf))
    return NULL;

  if (dynbuf->file_map)
    {
      rc_errno_set (EPERM);
      DBG_MSG ("Cannot reallocate mmap()'d file!\n");

      return NULL;
    }

  len = sizeof (char) * (dynbuf->wr_index + needed + 1);

  if (dynbuf->length < len)
    {
      char *new_ptr;

      /* Increase size in chunks to minimize reallocations */
      if (len < (dynbuf->length + DYNAMIC_BUFFER_SIZE))
	len = dynbuf->length + DYNAMIC_BUFFER_SIZE;

      new_ptr = xrealloc (dynbuf->data, len);
      if (NULL == new_ptr)
	return NULL;

      dynbuf->data = new_ptr;
      dynbuf->length = len;
    }

  return dynbuf;
}

void
rc_dynbuf_free (rc_dynbuf_t *dynbuf)
{
  if (NULL == dynbuf)
    return;

  if (!dynbuf->file_map)
    {
      if (NULL != dynbuf->data)
	{
	  free (dynbuf->data);
	  dynbuf->data = NULL;
	}
    }
  else
    {
      rc_file_unmap (dynbuf->data, dynbuf->length);
    }

  dynbuf->length = 0;
  dynbuf->rd_index = 0;
  dynbuf->wr_index = 0;

  free (dynbuf);
  dynbuf = NULL;
}

int
rc_dynbuf_write (rc_dynbuf_t *dynbuf, const char *buf, size_t length)
{
  int len;

  if (!rc_check_arg_dynbuf (dynbuf))
    return -1;

  if (!check_arg_str (buf))
    return -1;

  if (dynbuf->file_map)
    {
      rc_errno_set (EPERM);
      DBG_MSG ("Cannot write to readonly mmap()'d file!\n");

      return -1;
    }

  if (NULL == rc_dynbuf_reallocate (dynbuf, length))
    {
      DBG_MSG ("Could not reallocate dynamic buffer!\n");
      return -1;
    }

  len = snprintf ((dynbuf->data + dynbuf->wr_index), length + 1, "%s", buf);

  /* If len is less than length, it means the string was shorter than
   * given length */
  if (length > len)
    length = len;

  if (0 < length)
    dynbuf->wr_index += length;

  if (-1 == length)
    {
      rc_errno_set (errno);
      DBG_MSG ("Failed to write to dynamic buffer!\n");
    }

  return length;
}

int rc_dynbuf_write_fd (rc_dynbuf_t *dynbuf, int fd, size_t length)
{
  size_t len;
  size_t total = 0;

  if (!rc_check_arg_dynbuf (dynbuf))
    return -1;

  if (!check_arg_fd (fd))
    return -1;

  if (dynbuf->file_map)
    {
      rc_errno_set (EPERM);
      DBG_MSG ("Cannot write to readonly mmap()'d file!\n");

      return -1;
    }

  if (NULL == rc_dynbuf_reallocate (dynbuf, length))
    {
      DBG_MSG ("Could not reallocate dynamic buffer!\n");
      return -1;
    }

  do
    {
      len = read (fd, (dynbuf->data + dynbuf->wr_index + total),
		  (length - total));
      if (-1 == len)
	{
	  if (EINTR == errno)
	    {
	      /* We were interrupted, continue reading */
	      errno = 0;
	      /* Make sure we retry */
	      len = 1;
	    }
	  else
	    {
	      rc_errno_set (errno);
	      break;
	    }
	}
      else
	{
	  total += len;
	}
    }
  while ((0 < len) && (total < len));

  if (0 < total)
    dynbuf->wr_index += total;

  dynbuf->data[dynbuf->wr_index] = '\0';

  if (-1 == len)
    /* XXX: We set errno in above while loop. */
    DBG_MSG ("Failed to write to dynamic buffer!\n");

  return (-1 == len) ? len : total;
}

int
rc_dynbuf_sprintf (rc_dynbuf_t *dynbuf, const char *format, ...)
{
  va_list arg1, arg2;
  char test_str[10];
  int needed, written = 0;

  if (!rc_check_arg_dynbuf (dynbuf))
    return -1;

  if (!check_arg_str (format))
    return -1;

  if (dynbuf->file_map)
    {
      rc_errno_set (EPERM);
      DBG_MSG ("Cannot write to readonly mmap()'d file!\n");

      return -1;
    }

  va_start (arg1, format);
  va_copy (arg2, arg1);

  /* XXX: Lame way to try and figure out how much space we need */
  needed = vsnprintf (test_str, sizeof (test_str), format, arg2);
  va_end (arg2);

  if (NULL == rc_dynbuf_reallocate (dynbuf, needed))
    {
      DBG_MSG ("Could not reallocate dynamic buffer!\n");
      return -1;
    }

  written = vsnprintf ((dynbuf->data + dynbuf->wr_index), needed + 1,
		       format, arg1);
  va_end (arg1);

  if (0 < written)
    dynbuf->wr_index += written;

  if (-1 == written)
    {
      rc_errno_set (errno);
      DBG_MSG ("Failed to write to dynamic buffer!\n");
    }

  return written;
}

int
rc_dynbuf_read (rc_dynbuf_t *dynbuf, char *buf, size_t length)
{
  int len = length;

  if (!rc_check_arg_dynbuf (dynbuf))
    return -1;

  if (!check_arg_ptr (buf))
    return -1;

  if (dynbuf->rd_index >= dynbuf->length)
    return 0;

  if (dynbuf->wr_index < (dynbuf->rd_index + length))
    len = dynbuf->wr_index - dynbuf->rd_index;

  len = snprintf (buf, len + 1, "%s", (dynbuf->data + dynbuf->rd_index));

  /* If len is less than length, it means the string was shorter than
   * given length */
  if (length > len)
    length = len;

  if (0 < length)
    dynbuf->rd_index += length;

  if (-1 == length)
    {
      rc_errno_set (errno);
      DBG_MSG ("Failed to write from dynamic buffer!\n");
    }

  return length;
}

int
rc_dynbuf_read_fd (rc_dynbuf_t *dynbuf, int fd, size_t length)
{
  size_t len;
  size_t total = 0;
  size_t max_read = length;

  if (!rc_check_arg_dynbuf (dynbuf))
    return -1;

  if (!check_arg_fd (fd))
    return -1;

  if (dynbuf->rd_index >= dynbuf->length)
    return 0;

  if (dynbuf->wr_index < (dynbuf->rd_index + length))
    max_read = dynbuf->wr_index - dynbuf->rd_index;

  do
    {
      len = write (fd, (dynbuf->data + dynbuf->rd_index + total),
		   (max_read - total));
      if (-1 == len)
	{
	  if (EINTR == errno)
	    {
	      /* We were interrupted, continue reading */
	      errno = 0;
	      /* Make sure we retry */
	      len = 1;
	    }
	  else
	    {
	      rc_errno_set (errno);
	      break;
	    }
	}
      else
	{
	  total += len;
	}
    }
  while ((0 < len) && (total < len));

  if (0 < total)
    dynbuf->rd_index += total;

  if (-1 == len)
    /* XXX: We set errno in above while loop. */
    DBG_MSG ("Failed to write from dynamic buffer!\n");

  return (-1 == len) ? len : total;
}

char *
rc_dynbuf_read_line (rc_dynbuf_t *dynbuf)
{
  char *buf = NULL;
  size_t count = 0;

  if (!rc_check_arg_dynbuf (dynbuf))
    return NULL;

  if (dynbuf->rd_index == dynbuf->wr_index)
    return NULL;

  for (count = dynbuf->rd_index; count < dynbuf->wr_index && dynbuf->data[count] != '\n'; count++);

  if (count <= dynbuf->wr_index)
    {
      buf = xstrndup ((dynbuf->data + dynbuf->rd_index),
		      (count - dynbuf->rd_index));
      if (NULL == buf)
	return NULL;

      dynbuf->rd_index = count;

      /* Also skip the '\n' .. */
      if (dynbuf->rd_index < dynbuf->wr_index)
	dynbuf->rd_index++;
    }

  return buf;
}

int
rc_dynbuf_replace_char (rc_dynbuf_t *dynbuf, const char old, const char new)
{
  int i;
  int count = 0;

  if (!rc_check_arg_dynbuf (dynbuf))
    return -1;

  if (0 == dynbuf->wr_index)
    return 0;

  for (i = 0; i < dynbuf->wr_index; i++)
    {
      if (old == dynbuf->data[i])
	{
	  dynbuf->data[i] = new;
	  count++;
	}
    }

  return count;
}

bool
rc_dynbuf_read_eof (rc_dynbuf_t *dynbuf)
{
  if (!rc_check_arg_dynbuf (dynbuf))
    return FALSE;

  if (dynbuf->rd_index >= dynbuf->wr_index)
    return TRUE;

  return FALSE;
}

inline bool
rc_check_dynbuf (rc_dynbuf_t *dynbuf)
{
  if ((NULL == dynbuf) || (NULL == dynbuf->data) || (0 == dynbuf->length))
    return FALSE;

  return TRUE;
}

inline bool
__rc_check_arg_dynbuf (rc_dynbuf_t *dynbuf, const char *file, const char *func,
		     size_t line)
{
  if (!rc_check_dynbuf (dynbuf))
    {
      rc_errno_set (EINVAL);

      debug_message (file, func, line, "Invalid dynamic buffer passed!\n");

      return FALSE;
    }

  return TRUE;
}

