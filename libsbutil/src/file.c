/*
 * file.c
 *
 * Miscellaneous file related macro's and functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "rcscripts/rcutil.h"
#include "rcscripts/util/str_list.h"

bool
rc_file_exists (const char *pathname)
{
  struct stat buf;
  int retval;

  if (!check_str (pathname))
    return false;

  retval = lstat (pathname, &buf);
  if (-1 != retval)
    retval = true;
  else
    retval = false;

  return retval;
}

bool
rc_is_file (const char *pathname, bool follow_link)
{
  struct stat buf;
  int retval;

  if (!check_str (pathname))
    return false;

  retval = follow_link ? stat (pathname, &buf) : lstat (pathname, &buf);
  if ((-1 != retval) && (S_ISREG (buf.st_mode)))
    retval = true;
  else
    retval = false;

  return retval;
}

bool
rc_is_dir (const char *pathname, bool follow_link)
{
  struct stat buf;
  int retval;

  if (!check_str (pathname))
    return false;

  retval = follow_link ? stat (pathname, &buf) : lstat (pathname, &buf);
  if ((-1 != retval) && (S_ISDIR (buf.st_mode)))
    retval = true;
  else
    retval = false;

  return retval;
}

off_t
rc_get_size (const char *pathname, bool follow_link)
{
  struct stat buf;
  int retval;

  if (!check_str (pathname))
    return 0;

  retval = follow_link ? stat (pathname, &buf) : lstat (pathname, &buf);
  if (-1 != retval)
    retval = buf.st_size;
  else
    retval = 0;

  return retval;
}

char **
rc_ls_dir (const char *pathname, bool hidden, bool sort)
{
  DIR *dp;
  struct dirent *dir_entry;
  char **dirlist = NULL;

  if (!check_arg_str (pathname))
    return NULL;

  if (!rc_is_dir (pathname, true))
    {
      /* XXX: Should we error here? */
      DBG_MSG ("'%s' is not a directory.\n", pathname);
      return NULL;
    }

  dp = opendir (pathname);
  if (NULL == dp)
    {
      rc_errno_set (errno);
      DBG_MSG ("Failed to opendir() '%s'!\n", pathname);
      /* errno will be set by opendir() */
      goto error;
    }

  do
    {
      /* Clear errno to distinguish between EOF and error */
      errno = 0;
      dir_entry = readdir (dp);
      /* Only an error if 'errno' != 0, else EOF */
      if ((NULL == dir_entry) && (0 != errno))
	{
	  rc_errno_set (errno);
	  DBG_MSG ("Failed to readdir() '%s'!\n", pathname);
	  goto error;
	}
      if ((NULL != dir_entry)
	  /* Should we display hidden files? */
	  && (hidden ? 1 : dir_entry->d_name[0] != '.'))
	{
	  char *d_name = dir_entry->d_name;
	  char *str_ptr;

	  /* Do not list current or parent entries */
	  if ((0 == strcmp (d_name, ".")) || (0 == strcmp (d_name, "..")))
	    continue;

	  str_ptr = rc_strcatpaths (pathname, d_name);
	  if (NULL == str_ptr)
	    {
	      DBG_MSG ("Failed to allocate buffer!\n");
	      goto error;
	    }

	  if (sort)
	    str_list_add_item_sorted (dirlist, str_ptr, error);
	  else
	    str_list_add_item (dirlist, str_ptr, error);
	}
    }
  while (NULL != dir_entry);

  if (!check_strv (dirlist))
    {
      if (NULL != dirlist)
	str_list_free (dirlist);

      DBG_MSG ("Directory '%s' is empty.\n", pathname);
    }

  closedir (dp);

  return dirlist;

error:
  /* Free dirlist on error */
  str_list_free (dirlist);

  if (NULL != dp)
      closedir (dp);

  return NULL;
}


/*
 * Below two functions (rc_file_map and rc_file_unmap) are
 * from udev-050 (udev_utils.c).
 * (Some are slightly modified, please check udev for originals.)
 *
 * Copyright 2004 Kay Sievers <kay@vrfy.org>
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License as published by the
 *	Free Software Foundation version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

int
rc_file_map (const char *filename, char **buf, size_t * bufsize)
{
  struct stat stats;
  int fd;

  fd = open (filename, O_RDONLY);
  if (fd < 0)
    {
      rc_errno_set (errno);
      DBG_MSG ("Failed to open file!\n");
      return -1;
    }

  if (fstat (fd, &stats) < 0)
    {
      rc_errno_set (errno);
      DBG_MSG ("Failed to stat file!\n");

      close (fd);

      return -1;
    }

  if (0 == stats.st_size)
    {
      rc_errno_set (EINVAL);
      DBG_MSG ("Failed to mmap file with 0 size!\n");

      close (fd);

      return -1;
    }

  *buf = mmap (NULL, stats.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (*buf == MAP_FAILED)
    {
      rc_errno_set (errno);
      DBG_MSG ("Failed to mmap file!\n");

      close (fd);

      return -1;
    }
  *bufsize = stats.st_size;

  close (fd);

  return 0;
}

void
rc_file_unmap (char *buf, size_t bufsize)
{
  munmap (buf, bufsize);
}
