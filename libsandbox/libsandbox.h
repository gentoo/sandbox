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
#define __SB_SAFE(test) \
	(!is_sandbox_on() || (test))

#define _SB_SAFE_AT(_nr, _name, _dirfd, _path, _flags) \
       __SB_SAFE(before_syscall(_dirfd, _nr, _name, _path, _flags))
#define  SB_SAFE_AT(_dirfd, _path, _flags) \
        _SB_SAFE_AT(WRAPPER_NR, STRING_NAME, _dirfd, _path, _flags)
#define _SB_SAFE(_nr, _name, _path) \
        _SB_SAFE_AT(_nr, _name, AT_FDCWD, _path, 0)
#define  SB_SAFE(_path) \
         SB_SAFE_AT(AT_FDCWD, _path, 0)

#define _SB_SAFE_ACCESS_AT(_nr, _name, _dirfd, _path, _flags) \
       __SB_SAFE(before_syscall_access(_dirfd, _nr, _name, _path, _flags))
#define  SB_SAFE_ACCESS_AT(_dirfd, _path, _flags) \
        _SB_SAFE_ACCESS_AT(WRAPPER_NR, STRING_NAME, _dirfd, _path, _flags)
#define _SB_SAFE_ACCESS(_nr, _name, _path, _flags) \
        _SB_SAFE_ACCESS_AT(_nr, _name, AT_FDCWD, _path, _flags)
#define  SB_SAFE_ACCESS(_path, _flags) \
         SB_SAFE_ACCESS_AT(AT_FDCWD, _path, _flags)

#define _SB_SAFE_OPEN_INT_AT(_nr, _name, _dirfd, _path, _flags) \
       __SB_SAFE(before_syscall_open_int(_dirfd, _nr, _name, _path, _flags))
#define  SB_SAFE_OPEN_INT_AT(_dirfd, _path, _flags) \
        _SB_SAFE_OPEN_INT_AT(WRAPPER_NR, STRING_NAME, _dirfd, _path, _flags)
#define _SB_SAFE_OPEN_INT(_nr, _name, _path, _flags) \
        _SB_SAFE_OPEN_INT_AT(_nr, _name, AT_FDCWD, _path, _flags)
#define  SB_SAFE_OPEN_INT(_path, _flags) \
         SB_SAFE_OPEN_INT_AT(AT_FDCWD, _path, _flags)

#define  SB_SAFE_OPEN_CHAR_AT(_dirfd, _path, _mode) \
       __SB_SAFE(before_syscall_open_char(_dirfd, WRAPPER_NR, STRING_NAME, _path, _mode))
#define  SB_SAFE_OPEN_CHAR(_path, _mode) \
         SB_SAFE_OPEN_CHAR_AT(AT_FDCWD, _path, _mode)

bool is_sandbox_on(void);
bool before_syscall(int, int, const char *, const char *, int);
bool before_syscall_access(int, int, const char *, const char *, int);
bool before_syscall_open_int(int, int, const char *, const char *, int);
bool before_syscall_open_char(int, int, const char *, const char *, const char *);

extern char sandbox_lib[SB_PATH_MAX];
extern bool sandbox_on;
extern pid_t trace_pid;

extern void sb_lock(void);
extern void sb_unlock(void);

void trace_main(const char *filename, char *const argv[]);

__attribute__((__format__(__printf__, 1, 2))) void sb_eqawarn(const char *format, ...);
void sb_dump_backtrace(void);
__attribute__((noreturn)) void sb_abort(void);

/* glibc modified realpath() function */
char *erealpath(const char *, char *);
char *egetcwd(char *, size_t);
int canonicalize(const char *, char *);
/* most linux systems use ENAMETOOLONG, but some (ia64) use ERANGE, as do some BSDs */
#define errno_is_too_long() (errno == ENAMETOOLONG || errno == ERANGE)

#include "sbutil.h"

/* glibc sometimes redefines this crap on us */
#undef strdup
/* our helper xstrdup will be calling glibc strdup, so blah */
#undef xstrdup
#define xstrdup strdup

#endif /* __LIBSANDBOX_H__ */
