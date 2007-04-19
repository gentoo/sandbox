/*
 * file.c
 *
 * Miscellaneous file related macro's and functions.
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
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include "rcscripts/rcutil.h"

bool
rc_file_exists (const char *pathname)
{
  struct stat buf;
  int retval;

  if (!check_str (pathname))
    return FALSE;

  retval = lstat (pathname, &buf);
  if (-1 != retval)
    retval = TRUE;
  else
    retval = FALSE;

  return retval;
}

bool
rc_is_file (const char *pathname, bool follow_link)
{
  struct stat buf;
  int retval;

  if (!check_str (pathname))
    return FALSE;

  retval = follow_link ? stat (pathname, &buf) : lstat (pathname, &buf);
  if ((-1 != retval) && (S_ISREG (buf.st_mode)))
    retval = TRUE;
  else
    retval = FALSE;

  return retval;
}

bool
rc_is_link (const char *pathname)
{
  struct stat buf;
  int retval;

  if (!check_str (pathname))
    return FALSE;

  retval = lstat (pathname, &buf);
  if ((-1 != retval) && (S_ISLNK (buf.st_mode)))
    retval = TRUE;
  else
    retval = FALSE;

  return retval;
}

bool
rc_is_dir (const char *pathname, bool follow_link)
{
  struct stat buf;
  int retval;

  if (!check_str (pathname))
    return FALSE;

  retval = follow_link ? stat (pathname, &buf) : lstat (pathname, &buf);
  if ((-1 != retval) && (S_ISDIR (buf.st_mode)))
    retval = TRUE;
  else
    retval = FALSE;

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

time_t
rc_get_mtime (const char *pathname, bool follow_link)
{
  struct stat buf;
  int retval;

  if (!check_str (pathname))
    return 0;

  retval = follow_link ? stat (pathname, &buf) : lstat (pathname, &buf);
  if (-1 != retval)
    retval = buf.st_mtime;
  else
    retval = 0;

  return retval;
}

#if !defined(HAVE_REMOVE)
int
remove (const char *pathname)
{
  int retval;

  if (!check_arg_str (pathname))
    return -1;

  if (rc_is_dir (pathname, FALSE))
    retval = rmdir (pathname);
  else
    retval = unlink (pathname);

  if (0 != errno)
    {
      rc_errno_set (errno);
      DBG_MSG ("Failed to remove() '%s'!\n", pathname);
    }

  return retval;
}
#endif

int
rc_mktree (const char *pathname, mode_t mode)
{
  char *temp_name = NULL;
  char *temp_token = NULL;
  char *token_p;
  char *token;
  int retval;
  int lenght;

  if (!check_arg_str (pathname))
    return -1;

  /* Lenght of 'pathname' + extra for "./" if needed */
  lenght = strlen (pathname) + 2;
  /* lenght + '\0' */
  temp_name = xmalloc (lenght + 1);
  if (NULL == temp_name)
    return -1;

  temp_token = xstrndup (pathname, strlen (pathname));
  if (NULL == temp_token)
    goto error;

  token_p = temp_token;

  if (pathname[0] == '/')
    temp_name[0] = '\0';
  else
    /* If not an absolute path, make it local */
    strncpy (temp_name, ".", lenght);

  token = strsep (&token_p, "/");
  /* First token might be "", but that is OK as it will be when the
   * pathname starts with '/' */
  while (NULL != token)
    {
      strncat (temp_name, "/", lenght - strlen (temp_name));
      strncat (temp_name, token, lenght - strlen (temp_name));

      /* If it does not exist, create the dir.  If it does exit,
       * but is not a directory, we will catch it below. */
      if (!rc_file_exists (temp_name))
	{
	  retval = mkdir (temp_name, mode);
	  if (-1 == retval)
	    {
	      rc_errno_set (errno);
	      DBG_MSG ("Failed to create directory '%s'!\n", temp_name);
	      goto error;
	    }
	  /* Not a directory or symlink pointing to a directory */
	}
      else if (!rc_is_dir (temp_name, TRUE))
	{
	  rc_errno_set (ENOTDIR);
	  DBG_MSG ("Component in '%s' is not a directory!\n", temp_name);
	  goto error;
	}

      do
	{
	  token = strsep (&token_p, "/");
	  /* The first "" was Ok, but rather skip double '/' after that */
	}
      while ((NULL != token) && (0 == strlen (token)));
    }

  free (temp_name);
  free (temp_token);

  return 0;

error:
  free (temp_name);
  free (temp_token);

  return -1;
}

int
rc_rmtree (const char *pathname)
{
  char **dirlist = NULL;
  int i = 0;

  if (!check_arg_str (pathname))
    return -1;

  if (!rc_file_exists (pathname))
    {
      rc_errno_set (ENOENT);
      DBG_MSG ("'%s' does not exist!\n", pathname);
      return -1;
    }

  if (!rc_is_dir (pathname, FALSE))
    {
      rc_errno_set (ENOTDIR);
      DBG_MSG ("'%s' is not a directory!\n", pathname);
      return -1;
    }


  dirlist = rc_ls_dir (pathname, TRUE, FALSE);
  if ((NULL == dirlist) && (rc_errno_is_set ()))
    {
      /* Do not error out - caller should decide itself if it
       * it is an issue */
      DBG_MSG ("Failed to ls_dir() directory '%s'!\n", pathname);
      return -1;
    }

  while ((NULL != dirlist) && (NULL != dirlist[i]))
    {
      /* If it is a directory, call rc_rmtree() again with
       * it as argument */
      if (rc_is_dir (dirlist[i], FALSE))
	{
	  if (-1 == rc_rmtree (dirlist[i]))
	    {
	      DBG_MSG ("Failed to rm_tree() directory '%s'!\n", dirlist[i]);
	      goto error;
	    }
	}

      /* Now actually remove it.  Note that if it was a directory,
       * it should already be removed by above rc_rmtree() call */
      if ((rc_file_exists (dirlist[i]) && (-1 == remove (dirlist[i]))))
	{
	  rc_errno_set (errno);
	  DBG_MSG ("Failed to remove() '%s'!\n", dirlist[i]);
	  goto error;
	}
      i++;
    }

  str_list_free (dirlist);

  /* Now remove the parent */
  if (-1 == remove (pathname))
    {
      rc_errno_set (errno);
      DBG_MSG ("Failed to remove '%s'!\n", pathname);
      goto error;
    }

  return 0;
error:
  str_list_free (dirlist);

  return -1;
}

char **
rc_ls_dir (const char *pathname, bool hidden, bool sort)
{
  DIR *dp;
  struct dirent *dir_entry;
  char **dirlist = NULL;

  if (!check_arg_str (pathname))
    return NULL;

  if (!rc_is_dir (pathname, TRUE))
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

