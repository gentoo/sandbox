/*
 * debug.h
 *
 * Simle debugging/logging macro's and functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
 */

#ifndef __RC_DEBUG_H__
#define __RC_DEBUG_H__

#include <errno.h>
#include <stdio.h>

void
rc_log_domain (const char *new_domain);

/* Using errno to try and create a debug system that will 'trace' from the
 * point the initial error occured, is highly messy and needs strict errno
 * accounting.  So we rather implement our own private errno system.
 * Currently it still is a form of errno (I'd rather move to something less
 * messy, but it can be changed after I have a better idea of what exactly is
 * needed), as its easier to map to the normal errno system, but without the
 * issue of needing to keep in mind what others might do with errno's value,
 * and slightly less of an accounting nightmare. */
void
rc_errno_set (int rc_errno);
void
rc_errno_clear (void);
int
rc_errno_get (void);
bool
rc_errno_is_set (void);

#define rc_errno_save()		int _rc_old_errno = rc_errno_get ();
#define rc_errno_restore()	rc_errno_set (_rc_old_errno);

void
debug_message (const char *file, const char *func, int line,
	       const char *format, ...);

#define DBG_MSG(_format, _arg...) \
	debug_message (__FILE__, __func__, __LINE__, _format, ## _arg)

/*
 * Functions to check validity of some types.
 * They do not set errno.
 */

bool check_ptr (const void *ptr);
bool check_str (const char *str);
bool check_strv (char **str);
bool check_fd (int fd);
bool check_fp (FILE * fp);

/*
 * Functions and macro's to check validity of some types.
 * They DO set errno to EINVAL.
 */

bool __check_arg_ptr (const void *ptr, const char *file, const char *func, size_t line);
bool __check_arg_str (const char *str, const char *file, const char *func, size_t line);
bool __check_arg_strv (char **str, const char *file, const char *func, size_t line);
bool __check_arg_fd (int fd, const char *file, const char *func, size_t line);
bool __check_arg_fp (FILE * fp, const char *file, const char *func, size_t line);

#define check_arg_ptr(_ptr) \
 __check_arg_ptr (_ptr, __FILE__, __func__, __LINE__)
#define check_arg_str(_str) \
 __check_arg_str (_str, __FILE__, __func__, __LINE__)
#define check_arg_strv(_str) \
 __check_arg_strv (_str, __FILE__, __func__, __LINE__)
#define check_arg_fd(_fd) \
 __check_arg_fd (_fd, __FILE__, __func__, __LINE__)
#define check_arg_fp(_fp) \
 __check_arg_fp (_fp, __FILE__, __func__, __LINE__)

#endif /* __RC_DEBUG_H__ */
