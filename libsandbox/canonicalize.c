/* Return the canonical absolute name of a given file.
   Copyright (C) 1996-2001, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include "headers.h"
#include "sbutil.h"
#include "libsandbox.h"

#ifndef __set_errno
# define __set_errno(val) errno = (val)
#endif

/* Return the canonical absolute name of file NAME.  A canonical name
   does not contain any `.', `..' components nor any repeated path
   separators ('/') or symlinks.  All path components must exist.  If
   RESOLVED is null, the result is malloc'd; otherwise, if the
   canonical name is SB_PATH_MAX chars or more, returns null with `errno'
   set to ENAMETOOLONG; if the name fits in fewer than SB_PATH_MAX chars,
   returns the name in RESOLVED.  If the name cannot be resolved and
   RESOLVED is non-NULL, it contains the path of the first component
   that cannot be resolved.  If the path can be resolved, RESOLVED
   holds the same value as the value returned.	*/

/* Modified: 19 Aug 2002; Martin Schlemmer <azarah@gentoo.org>
 *
 *  Cleaned up unneeded stuff, and change so that it will not
 *  resolve symlinks.  Also prepended a 'e' to functions that
 *  I did not rip out.
 *
 */

char *
erealpath(const char *name, char *resolved)
{
	char *rpath, *dest, *recover;
	const char *start, *end, *rpath_limit;
	long int path_max;

	if (name == NULL) {
		/* As per Single Unix Specification V2 we must return an error if
		   either parameter is a null pointer.	We extend this to allow
		   the RESOLVED parameter to be NULL in case the we are expected to
		   allocate the room for the return value.  */
		__set_errno(EINVAL);
		return NULL;
	}

	if (name[0] == '\0') {
		/* As per Single Unix Specification V2 we must return an error if
		   the name argument points to an empty string.  */
		__set_errno(ENOENT);
		return NULL;
	}
#ifdef SB_PATH_MAX
	path_max = SB_PATH_MAX;
#else
	path_max = pathconf(name, _PC_PATH_MAX);
	if (path_max <= 0)
		path_max = 1024;
#endif

	if (resolved == NULL) {
		rpath = xmalloc(path_max);
	} else
		rpath = resolved;
	rpath_limit = rpath + path_max;

	recover = NULL;
	if (name[0] != '/') {
		if (!egetcwd(rpath, path_max)) {
			rpath[0] = '\0';
			goto error;
		}

		/* Can we actually access the working dir (sane perms) ?
		 * If not, try a little harder to consume this path in
		 * case it has symlinks out into a better world ...
		 */
		struct stat st;
		if (lstat(rpath, &st) == -1 && errno == EACCES) {
			char *p = rpath;
			strcpy(rpath, name);
			do {
				p = strchr(p, '/');
				if (p) *p = '\0';
				if (lstat(rpath, &st))
					break;
				if (S_ISLNK(st.st_mode)) {
					ssize_t cnt = readlink(rpath, rpath, path_max);
					if (cnt == -1)
						break;
					rpath[cnt] = '\0';
					if (p) {
						size_t bytes_left = strlen(p);
						if (bytes_left >= path_max)
							break;
						strncat(rpath, name + (p - rpath + 1),
							path_max - bytes_left - 1);
					}

					/* Ok, we have a chance at something better.  If
					 * this fails, we give up.  Otherwise we set things
					 * to match the new paths that we've found and hook
					 * back into the normal process.
					 */
					recover = erealpath(rpath, NULL);
					if (recover) {
						if (recover[0] != '/') {
							strcpy(rpath, recover);
							break;
						} else {
							name = recover;
							goto recover_full;
						}
					} else
						goto error;
				}
				if (p) *p++ = '/';
			} while (p);
		}

		dest = strchr(rpath, '\0');
	} else {
 recover_full:
		rpath[0] = '/';
		dest = rpath + 1;
	}

	for (start = end = name; *start; start = end) {
		/* Skip sequence of multiple path-separators.  */
		while (*start == '/')
			++start;

		/* Find end of path component.	*/
		for (end = start; *end && *end != '/'; ++end)
			/* Nothing.  */ ;

		if (end - start == 0)
			break;
		else if (end - start == 1 && start[0] == '.')
			/* nothing */ ;
		else if (end - start == 2 && start[0] == '.' && start[1] == '.') {
			/* Back up to previous component, ignore if at root already.  */
			if (dest > rpath + 1)
				while ((--dest)[-1] != '/') ;
		} else {
			size_t new_size;

			if (dest[-1] != '/')
				*dest++ = '/';

			if (dest + (end - start) >= rpath_limit) {
				ptrdiff_t dest_offset = dest - rpath;
				char *new_rpath;

				if (resolved) {
					__set_errno(ENAMETOOLONG);
					if (dest > rpath + 1)
						dest--;
					*dest = '\0';
					goto error;
				}
				new_size = rpath_limit - rpath;
				if (end - start + 1 > path_max)
					new_size += end - start + 1;
				else
					new_size += path_max;
				new_rpath = (char *) xrealloc(rpath, new_size);
				rpath = new_rpath;
				rpath_limit = rpath + new_size;

				dest = rpath + dest_offset;
			}

			memcpy(dest, start, end - start);
			dest += end - start;
			*dest = '\0';
		}
	}
#if 1
	if (dest > rpath + 1 && dest[-1] == '/')
		--dest;
#endif
	*dest = '\0';

	return rpath;

error:
	if (resolved)
		snprintf(resolved, path_max, "%s", rpath);
	else
		free(rpath);
	free(recover);
	return NULL;
}

// vim:noexpandtab noai:cindent ai
