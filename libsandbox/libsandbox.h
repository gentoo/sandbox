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

/* glibc sometimes redefines this crap on us */
#undef strdup

/* Macros to check if a function should be executed */
#define FUNCTION_SANDBOX_SAFE_AT(_dirfd, _path) \
	((0 == is_sandbox_on()) || (1 == before_syscall(_dirfd, STRING_NAME, _path)))
#define FUNCTION_SANDBOX_SAFE(_path) \
        FUNCTION_SANDBOX_SAFE_AT(AT_FDCWD, _path)

#define FUNCTION_SANDBOX_SAFE_ACCESS_AT(_dirfd, _path, _flags) \
	((0 == is_sandbox_on()) || (1 == before_syscall_access(_dirfd, STRING_NAME, _path, _flags)))
#define FUNCTION_SANDBOX_SAFE_ACCESS(_path, _flags) \
        FUNCTION_SANDBOX_SAFE_ACCESS_AT(AT_FDCWD, _path, _flags)

#define FUNCTION_SANDBOX_SAFE_OPEN_INT_AT(_dirfd, _path, _flags) \
	((0 == is_sandbox_on()) || (1 == before_syscall_open_int(_dirfd, STRING_NAME, _path, _flags)))
#define FUNCTION_SANDBOX_SAFE_OPEN_INT(_path, _flags) \
        FUNCTION_SANDBOX_SAFE_OPEN_INT_AT(AT_FDCWD, _path, _flags)

#define FUNCTION_SANDBOX_SAFE_OPEN_CHAR_AT(_dirfd, _path, _mode) \
	((0 == is_sandbox_on()) || (1 == before_syscall_open_char(_dirfd, STRING_NAME, _path, _mode)))
#define FUNCTION_SANDBOX_SAFE_OPEN_CHAR(_path, _mode) \
        FUNCTION_SANDBOX_SAFE_OPEN_CHAR_AT(AT_FDCWD, _path, _mode)

int canonicalize(const char *, char *);

int is_sandbox_on(void);
int before_syscall(int, const char *, const char *);
int before_syscall_access(int, const char *, const char *, int);
int before_syscall_open_int(int, const char *, const char *, int);
int before_syscall_open_char(int, const char *, const char *, const char *);

/* glibc modified realpath() function */
char *erealpath(const char *, char *);
char *egetcwd(char *, size_t);

#endif /* __LIBSANDBOX_H__ */
