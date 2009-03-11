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
#define _FUNCTION_SANDBOX_SAFE(test) \
	(!is_sandbox_on() || (test))

#define FUNCTION_SANDBOX_SAFE_AT(_dirfd, _path, _flags) \
       _FUNCTION_SANDBOX_SAFE(before_syscall(_dirfd, WRAPPER_NR, STRING_NAME, _path, _flags))
#define FUNCTION_SANDBOX_SAFE(_path) \
        FUNCTION_SANDBOX_SAFE_AT(AT_FDCWD, _path, 0)

#define FUNCTION_SANDBOX_SAFE_ACCESS_AT(_dirfd, _path, _flags) \
       _FUNCTION_SANDBOX_SAFE(before_syscall_access(_dirfd, WRAPPER_NR, STRING_NAME, _path, _flags))
#define FUNCTION_SANDBOX_SAFE_ACCESS(_path, _flags) \
        FUNCTION_SANDBOX_SAFE_ACCESS_AT(AT_FDCWD, _path, _flags)

#define FUNCTION_SANDBOX_SAFE_OPEN_INT_AT(_dirfd, _path, _flags) \
       _FUNCTION_SANDBOX_SAFE(before_syscall_open_int(_dirfd, WRAPPER_NR, STRING_NAME, _path, _flags))
#define FUNCTION_SANDBOX_SAFE_OPEN_INT(_path, _flags) \
        FUNCTION_SANDBOX_SAFE_OPEN_INT_AT(AT_FDCWD, _path, _flags)

#define FUNCTION_SANDBOX_SAFE_OPEN_CHAR_AT(_dirfd, _path, _mode) \
       _FUNCTION_SANDBOX_SAFE(before_syscall_open_char(_dirfd, WRAPPER_NR, STRING_NAME, _path, _mode))
#define FUNCTION_SANDBOX_SAFE_OPEN_CHAR(_path, _mode) \
        FUNCTION_SANDBOX_SAFE_OPEN_CHAR_AT(AT_FDCWD, _path, _mode)

bool is_sandbox_on(void);
bool before_syscall(int, int, const char *, const char *, int);
bool before_syscall_access(int, int, const char *, const char *, int);
bool before_syscall_open_int(int, int, const char *, const char *, int);
bool before_syscall_open_char(int, int, const char *, const char *, const char *);

extern char sandbox_lib[SB_PATH_MAX];
extern volatile bool sandbox_on;

void sb_dump_backtrace(void);

/* glibc modified realpath() function */
char *erealpath(const char *, char *);
char *egetcwd(char *, size_t);
int canonicalize(const char *, char *);

#endif /* __LIBSANDBOX_H__ */
