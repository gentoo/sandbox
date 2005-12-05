/* 
 * Copyright (C) 2005 Martin Schlemmer <azarah@gentoo.org>,
 * Distributed under the terms of the GNU General Public License, v2 or later 
 * Author: Martin Schlemmer <azarah@gentoo.org>
 *    
 * $Header$
 */

#ifndef __LIBSANDBOX_H__
#define __LIBSANDBOX_H__

#if !defined(OUTSIDE_LIBSANDBOX) && !defined(HAVE_RTLD_NEXT)
/* Need to disable sandbox, as on non-RTLD_NEXT libc's, opendir()
 * used by some getcwd() implementations resolves to the sandbox
 * opendir() wrapper ... */
extern int sandbox_on;
# define set_sandbox_on		sandbox_on = 1
# define set_sandbox_off	sandbox_on = 0
#else
# define set_sandbox_on
# define set_sandbox_off
#endif /* OUTSIDE_LIBSANDBOX */
	
#endif /* __LIBSANDBOX_H__ */
