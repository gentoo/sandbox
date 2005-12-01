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
	
char *egetcwd(char *buf, size_t size)
{
	struct stat st;
	char *tmpbuf;

	__set_errno(0);
	tmpbuf = getcwd(buf, size);
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

