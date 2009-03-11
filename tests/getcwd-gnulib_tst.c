/* This comes from gnulib: gnulib/m4/getcwd-path-max.m4
 * On glibc systems, we currently exit(1).  Sandbox should not get in the
 * way of this, nor should it crash.
 */

#include "tests.h"

#ifndef S_IRWXU
# define S_IRWXU 0700
#endif

/* The length of this name must be 8.  */
#define DIR_NAME "confdir3"
#define DIR_NAME_LEN 8
#define DIR_NAME_SIZE (DIR_NAME_LEN + 1)

/* The length of "../".  */
#define DOTDOTSLASH_LEN 3

/* Leftover bytes in the buffer, to work around library or OS bugs.  */
#define BUF_SLOP 20

int
main ()
{
  char buf[PATH_MAX * (DIR_NAME_SIZE / DOTDOTSLASH_LEN + 1)
	   + DIR_NAME_SIZE + BUF_SLOP];
  char *cwd = getcwd (buf, PATH_MAX);
  size_t initial_cwd_len;
  size_t cwd_len;
  int fail = 0;
  size_t n_chdirs = 0;

  if (cwd == NULL)
    exit (1);

  cwd_len = initial_cwd_len = strlen (cwd);

  while (1)
    {
      size_t dotdot_max = PATH_MAX * (DIR_NAME_SIZE / DOTDOTSLASH_LEN);
      char *c = NULL;

      cwd_len += DIR_NAME_SIZE;
      /* If mkdir or chdir fails, it could be that this system cannot create
	 any file with an absolute name longer than PATH_MAX, such as cygwin.
	 If so, leave fail as 0, because the current working directory can't
	 be too long for getcwd if it can't even be created.  For other
	 errors, be pessimistic and consider that as a failure, too.  */
      if (mkdir (DIR_NAME, S_IRWXU) < 0 || chdir (DIR_NAME) < 0)
	{
	  if (! (errno == ERANGE || ENAMETOOLONG == errno))
	    fail = 2;
	  break;
	}

      if (PATH_MAX <= cwd_len && cwd_len < PATH_MAX + DIR_NAME_SIZE)
	{
	  c = getcwd (buf, PATH_MAX);
	  if (!c && errno == ENOENT)
	    {
	      fail = 1;
	      break;
	    }
	  if (c || ! (errno == ERANGE || ENAMETOOLONG == errno))
	    {
	      fail = 2;
	      break;
	    }
	}

      if (dotdot_max <= cwd_len - initial_cwd_len)
	{
	  if (dotdot_max + DIR_NAME_SIZE < cwd_len - initial_cwd_len)
	    break;
	  c = getcwd (buf, cwd_len + 1);
	  if (!c)
	    {
	      if (! (errno == ERANGE || errno == ENOENT
		     || ENAMETOOLONG == errno))
		{
		  fail = 2;
		  break;
		}
	      if (AT_FDCWD || errno == ERANGE || errno == ENOENT)
		{
		  fail = 1;
		  break;
		}
	    }
	}

      if (c && strlen (c) != cwd_len)
	{
	  fail = 2;
	  break;
	}
      ++n_chdirs;
    }

  /* Leaving behind such a deep directory is not polite.
     So clean up here, right away, even though the driving
     shell script would also clean up.  */
  {
    size_t i;

    /* Try rmdir first, in case the chdir failed.  */
    rmdir (DIR_NAME);
    for (i = 0; i <= n_chdirs; i++)
      {
	if (chdir ("..") < 0)
	  break;
	if (rmdir (DIR_NAME) != 0)
	  break;
      }
  }

  exit (fail);
}
