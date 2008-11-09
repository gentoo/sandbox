/*
 * libsandbox.h
 *
 * Defines related to libsandbox.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#ifndef __LIBSANDBOX_H__
#define __LIBSANDBOX_H__

/* Macros to check if a function should be executed */
#define FUNCTION_SANDBOX_SAFE(_func, _path) \
	((0 == is_sandbox_on()) || (1 == before_syscall(_func, _path)))

#define FUNCTION_SANDBOX_SAFE_ACCESS(_func, _path, _flags) \
	((0 == is_sandbox_on()) || (1 == before_syscall_access(_func, _path, _flags)))

#define FUNCTION_SANDBOX_SAFE_OPEN_INT(_func, _path, _flags) \
	((0 == is_sandbox_on()) || (1 == before_syscall_open_int(_func, _path, _flags)))

#define FUNCTION_SANDBOX_SAFE_OPEN_CHAR(_func, _path, _mode) \
	((0 == is_sandbox_on()) || (1 == before_syscall_open_char(_func, _path, _mode)))

int canonicalize(const char *, char *);

int is_sandbox_on();
int before_syscall(const char *, const char *);
int before_syscall_access(const char *, const char *, int);
int before_syscall_open_int(const char *, const char *, int);
int before_syscall_open_char(const char *, const char *, const char *);

/* glibc modified realpath() function */
char *erealpath(const char *, char *);
char *egetcwd(char *, size_t);

#endif /* __LIBSANDBOX_H__ */
