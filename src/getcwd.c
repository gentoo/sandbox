/* These functions find the absolute path to the current working directory.  */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "localdecls.h"

#ifndef __set_errno
# define __set_errno(val) errno = (val)
#endif

#if !defined(OUTSIDE_LIBSANDBOX)
extern int sandbox_on;
# define set_sandbox_on		sandbox_on = 1
# define set_sandbox_off	sandbox_on = 0
#else
# define set_sandbox_on
# define set_sandbox_off
#endif /* OUTSIDE_LIBSANDBOX */
	
char *egetcwd(char *buf, size_t size)
{
	struct stat st;
	char *tmpbuf;

	__set_errno(0);
	set_sandbox_off;
	tmpbuf = getcwd(buf, size);
	set_sandbox_on;
	if (tmpbuf)
		lstat(buf, &st);

	if ((tmpbuf) && (errno == ENOENT)) {
		/* If lstat() failed with eerror = ENOENT, then its
		 * possible that we are running on an older kernel
		 * which had issues with returning invalid paths if
		 * they got too long.
		 */
		free(tmpbuf);
			
		return NULL;
	}

	return tmpbuf;
}

