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
       __SB_SAFE(before_syscall(_nr, _name, _dirfd, _path, _flags))
#define  SB_SAFE_AT(_dirfd, _path, _flags) \
        _SB_SAFE_AT(WRAPPER_NR, STRING_NAME, _dirfd, _path, _flags)
#define _SB_SAFE(_nr, _name, _path) \
        _SB_SAFE_AT(_nr, _name, AT_FDCWD, _path, 0)
#define  SB_SAFE(_path) \
         SB_SAFE_AT(AT_FDCWD, _path, 0)

#define _SB_SAFE_ACCESS_AT(_nr, _name, _dirfd, _path, _mode, _flags) \
       __SB_SAFE(before_syscall_access(_nr, _name, _dirfd, _path, _mode, _flags))
#define  SB_SAFE_ACCESS_AT(_dirfd, _path, _mode, _flags) \
        _SB_SAFE_ACCESS_AT(WRAPPER_NR, STRING_NAME, _dirfd, _path, _mode, _flags)
#define _SB_SAFE_ACCESS(_nr, _name, _path, _mode) \
        _SB_SAFE_ACCESS_AT(_nr, _name, AT_FDCWD, _path, _mode, 0)
#define  SB_SAFE_ACCESS(_path, _mode) \
         SB_SAFE_ACCESS_AT(AT_FDCWD, _path, _mode, 0)

#define _SB_SAFE_OPEN_INT_AT(_nr, _name, _dirfd, _path, _flags) \
       __SB_SAFE(before_syscall_open_int(_nr, _name, _dirfd, _path, _flags))
#define  SB_SAFE_OPEN_INT_AT(_dirfd, _path, _flags) \
        _SB_SAFE_OPEN_INT_AT(WRAPPER_NR, STRING_NAME, _dirfd, _path, _flags)
#define _SB_SAFE_OPEN_INT(_nr, _name, _path, _flags) \
        _SB_SAFE_OPEN_INT_AT(_nr, _name, AT_FDCWD, _path, _flags)
#define  SB_SAFE_OPEN_INT(_path, _flags) \
         SB_SAFE_OPEN_INT_AT(AT_FDCWD, _path, _flags)

#define  SB_SAFE_OPEN_CHAR_AT(_dirfd, _path, _mode) \
       __SB_SAFE(before_syscall_open_char(WRAPPER_NR, STRING_NAME, _dirfd, _path, _mode))
#define  SB_SAFE_OPEN_CHAR(_path, _mode) \
         SB_SAFE_OPEN_CHAR_AT(AT_FDCWD, _path, _mode)

#define _SB_SAFE_FD(_nr, _name, _fd) \
        __SB_SAFE(before_syscall_fd(_nr, _name, _fd))
#define  SB_SAFE_FD(_fd) \
         _SB_SAFE_FD(WRAPPER_NR, STRING_NAME, _fd)

/* Symbols that don't exist in the C library will be <= this value. */
#define SB_NR_UNDEF -99999
#define SB_NR_IS_DEFINED(nr) (nr > SB_NR_UNDEF)

bool is_sandbox_on(void);
bool before_syscall(int sb_nr, const char *func, int dirfd, const char *file, int flags);
bool before_syscall_access(int sb_nr, const char *func, int dirfd, const char *file, int mode, int flags);
bool before_syscall_open_int(int sb_nr, const char *func, int dirfd, const char *file, int flags);
bool before_syscall_open_char(int sb_nr, const char *func, int dirfd, const char *file, const char *mode);
bool before_syscall_fd(int sb_nr, const char *func, int fd);

enum sandbox_method_t get_sandbox_method(void);

void *get_dlsym(const char *symname, const char *symver);

extern char sandbox_lib[PATH_MAX];
extern bool sandbox_on;

struct sb_envp_ctx {
	/* Sandboxified environment with sandbox variables injected.
	 * Allocated by 'sb_new_envp', freed by 'sb_free_envp'. */
	char ** sb_envp;
	/* Original environment. Passed from outside. */
	char ** orig_envp;

	/* Internal counter to free.
	 * Not to be used outside sb_{new,free}_envp. */
	size_t __mod_cnt;
};
struct sb_envp_ctx sb_new_envp(char **envp, bool insert);
void sb_free_envp(struct sb_envp_ctx * envp_ctx);

extern pid_t trace_pid;

extern void sb_lock(void);
extern void sb_unlock(void);

bool trace_possible(const char *filename, char *const argv[], const void *data);
void trace_main(void);

bool sb_abspathat(int dirfd, const char *restrict path, char *buf, size_t bufsiz);
bool sb_realpathat(int dirfd, const char *restrict path, char *buf, size_t bufsiz, int flags, bool create);

/* most linux systems use ENAMETOOLONG, but some (ia64) use ERANGE, as do some BSDs */
#define errno_is_too_long() (errno == ENAMETOOLONG || errno == ERANGE)

size_t malloc_size(void *ptr);

#include "sbutil.h"

/* glibc sometimes redefines this crap on us */
#undef strdup
/* our helper xstrdup will be calling glibc strdup, so blah */
#undef xstrdup
#define xstrdup strdup

#endif /* __LIBSANDBOX_H__ */
