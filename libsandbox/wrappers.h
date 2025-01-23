/*
 * wrappers.h
 *
 * Function wrapping functions.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef __WRAPPERS_H__
#define __WRAPPERS_H__

/* Wrapper for internal use of functions in libsandbox.  Should really
 * autogenerate this header ...
 */
#define                sb_unwrapped_access sb_unwrapped_access_DEFAULT
attribute_hidden int   sb_unwrapped_access (const char *, int);
#define                sb_unwrapped_faccessat sb_unwrapped_faccessat_DEFAULT
attribute_hidden int   sb_unwrapped_faccessat (int, const char *, int, int);
#define                sb_unwrapped_getcwd sb_unwrapped_getcwd_DEFAULT
attribute_hidden char *sb_unwrapped_getcwd (char *, size_t);
#ifdef HAVE_OPEN64
#define                sb_unwrapped_open   sb_unwrapped_open64_DEFAULT
#else
#define                sb_unwrapped_open   sb_unwrapped_open_DEFAULT
#endif
attribute_hidden int   sb_unwrapped_open   (const char *, int, mode_t);
#ifdef HAVE_OPENAT64
#define                sb_unwrapped_openat   sb_unwrapped_openat64_DEFAULT
#else
#define                sb_unwrapped_openat   sb_unwrapped_openat_DEFAULT
#endif
attribute_hidden int   sb_unwrapped_openat (int dirfd, const char *path, int flags, mode_t mode);
#define                sb_unwrapped_popen  sb_unwrapped_popen_DEFAULT
attribute_hidden FILE *sb_unwrapped_popen  (const char *, const char *);

attribute_hidden bool sb_fopen_pre_check    (const char *func, const char *pathname, const char *mode);
attribute_hidden bool sb_fopen64_pre_check  (const char *func, const char *pathname, const char *mode);
attribute_hidden bool sb_mkdirat_pre_check  (const char *func, const char *pathname, int dirfd);
attribute_hidden bool sb_openat_pre_check   (const char *func, const char *pathname, int dirfd, int flags);
attribute_hidden bool sb_openat64_pre_check (const char *func, const char *pathname, int dirfd, int flags);
attribute_hidden bool sb_opendir_pre_check  (const char *func, const char *name);
attribute_hidden bool sb_unlinkat_pre_check (const char *func, const char *pathname, int dirfd);
attribute_hidden bool sb_common_at_pre_check(const char *func, const char **pathname, int dirfd,
                                             char *dirfd_path, size_t dirfd_path_len);

#endif
