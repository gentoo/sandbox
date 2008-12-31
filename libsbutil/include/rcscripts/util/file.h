/*
 * file.h
 *
 * Miscellaneous file related macro's and functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
 */

#ifndef __RC_FILE_H__
#define __RC_FILE_H__

#include <stdio.h>

/* The following functions do not care about errors - they only return
 * TRUE if 'pathname' exist, and is the type requested, or else FALSE.
 * This is only if pathname is valid ... */
bool rc_file_exists (const char *pathname);
bool rc_is_file (const char *pathname, bool follow_link);
bool rc_is_dir (const char *pathname, bool follow_link);

/* The following functions do not care about errors - it only returns
 * the size/mtime of 'pathname' if it exists, and is the type requested,
 * or else 0. */
off_t rc_get_size (const char *pathname, bool follow_link);

/* The following return a pointer on success, or NULL with errno set on error.
 * If it returned NULL, but errno is not set, then there was no error, but
 * there is nothing to return. */
char **rc_ls_dir (const char *pathname, bool hidden, bool sort);

/* Below two functions (rc_file_map and rc_file_unmap) are from
 * udev-050 (udev_utils.c).  Please see misc.c for copyright info.
 * (Some are slightly modified, please check udev for originals.) */
int rc_file_map (const char *filename, char **buf, size_t * bufsize);
void rc_file_unmap (char *buf, size_t bufsize);

#endif /* __RC_FILE_H__ */
