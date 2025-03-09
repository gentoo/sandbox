/*
 * sbutil.h
 *
 * Util defines.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 * Some parts might have Copyright:
 *   Copyright (C) 2002 Brad House <brad@mainstreetsoftworks.com>
 */

#ifndef __SBUTIL_H__
#define __SBUTIL_H__

#include "headers.h"
#include "include/rcscripts/rcutil.h"

#define SANDBOX_CONF_FILE      ETCDIR "/sandbox.conf"
#define SANDBOX_CONFD_DIR      ETCDIR "/sandbox.d"

#define LIB_NAME               "libsandbox.so"
#define BASHRC_NAME            "sandbox.bashrc"
#define TMPDIR                 "/tmp"
#define PORTAGE_TMPDIR         "/var/tmp/portage"
#define SANDBOX_LOG_LOCATION   "/var/log/sandbox"
#define LOG_FILE_PREFIX        "/sandbox-"
#define DEBUG_LOG_FILE_PREFIX  "/sandbox-debug-"
#define LOG_FILE_EXT           ".log"

#define ENV_LD_PRELOAD         "LD_PRELOAD"

#define ENV_TMPDIR             "TMPDIR"
#define ENV_PORTAGE_TMPDIR     "PORTAGE_TMPDIR"

#define ENV_BASH_ENV           "BASH_ENV"

#define ENV_NOCOLOR            "NOCOLOR"

#define ENV_SANDBOX_VERBOSE    "SANDBOX_VERBOSE"
#define ENV_SANDBOX_DEBUG      "SANDBOX_DEBUG"

#define ENV_SANDBOX_TESTING    "__SANDBOX_TESTING"

#define ENV_SANDBOX_LIB        "SANDBOX_LIB"
#define ENV_SANDBOX_BASHRC     "SANDBOX_BASHRC"
#define ENV_SANDBOX_LOG        "SANDBOX_LOG"
#define ENV_SANDBOX_DEBUG_LOG  "SANDBOX_DEBUG_LOG"
#define ENV_SANDBOX_MESSAGE_PATH "SANDBOX_MESSAGE_P@TH" /* @ is not a typo */
#define ENV_SANDBOX_WORKDIR    "SANDBOX_WORKDIR"

#define ENV_SANDBOX_DENY       "SANDBOX_DENY"
#define ENV_SANDBOX_READ       "SANDBOX_READ"
#define ENV_SANDBOX_WRITE      "SANDBOX_WRITE"
#define ENV_SANDBOX_PREDICT    "SANDBOX_PREDICT"

#define ENV_SANDBOX_METHOD     "SANDBOX_METHOD"
#define ENV_SANDBOX_ON         "SANDBOX_ON"

#define ENV_SANDBOX_ACTIVE     "SANDBOX_ACTIVE"
#define SANDBOX_ACTIVE         "armedandready"

extern const char *colors[];
#define COLOR_NORMAL           colors[0]
#define COLOR_GREEN            colors[1]
#define COLOR_YELLOW           colors[2]
#define COLOR_RED              colors[3]

char *get_sandbox_conf(void);
char *get_sandbox_confd(char *path);
void get_sandbox_lib(char *path);
void get_sandbox_rc(char *path);
void get_sandbox_log(char *path, const char *tmpdir);
void get_sandbox_debug_log(char *path, const char *tmpdir);
void get_sandbox_message_path(char *path);
int get_tmp_dir(char *path);
bool is_val_on(const char *);
bool is_val_off(const char *);
bool is_env_on(const char *);
bool is_env_off(const char *);
bool is_env_set_on(const char *, bool *);
bool is_env_set_off(const char *, bool *);
static inline bool is_env_var(const char *env, const char *var, size_t vlen)
{
	return !strncmp(env, var, vlen) && env[vlen] == '=';
}

typedef enum sandbox_method_t {
  SANDBOX_METHOD_ANY = 0,
  SANDBOX_METHOD_PRELOAD,
} sandbox_method_t;
sandbox_method_t parse_sandbox_method(const char *);
const char *str_sandbox_method(sandbox_method_t);

/* proc helpers */
extern const char sb_fd_dir[];
#define sb_get_fd_dir() sb_fd_dir
const char *sb_get_cmdline(pid_t pid);

/* libsandbox need to use a wrapper for open */
attribute_hidden extern int (*sbio_faccessat)(int, const char *, int, int);
attribute_hidden extern int (*sbio_open)(const char *, int, mode_t);
attribute_hidden extern FILE *(*sbio_popen)(const char *, const char *);
extern const char *sbio_message_path;
extern const char sbio_fallback_path[];
/* Convenience functions to reliably open, read and write to a file */
int sb_open(const char *path, int flags, mode_t mode);
size_t sb_read(int fd, void *buf, size_t count);
size_t sb_write(int fd, const void *buf, size_t count);
int sb_copy_file_to_fd(const char *file, int ofd);
int sb_exists(int dirfd, const char *pathname, int flags);
int sb_fstat(int fd, mode_t *mode, int64_t *size);

/* Reliable output */
__printf(1, 2) void sb_printf(const char *format, ...);
__printf(2, 3) void sb_fdprintf(int fd, const char *format, ...);
__printf(2, 0) void sb_vfdprintf(int fd, const char *format, va_list args);
__printf(3, 4) void sb_efunc(const char *color, const char *hilight, const char *format, ...);
__printf(1, 2) void sb_einfo(const char *format, ...);
__printf(1, 2) void sb_ewarn(const char *format, ...);
__printf(1, 2) void sb_eerror(const char *format, ...);
__printf(1, 2) void sb_eqawarn(const char *format, ...);
__printf(1, 2) void sb_debug_dyn(const char *format, ...);
__printf(1, 2) void sb_eraw(const char *format, ...);
__printf(4, 5) void __sb_ebort(const char *file, const char *func, size_t line_num, const char *format, ...) __noreturn;
#define sb_ebort(format, ...) __sb_ebort(__FILE__, __func__, __LINE__, format, ## __VA_ARGS__)
void sb_dump_backtrace(void);
void __sb_dump_backtrace(void);
void sb_gdb(void);
void sb_maybe_gdb(void);
#ifdef NDEBUG
#define sb_maybe_gdb()
#endif
#define sb_assert(cond) \
	do { \
		if (!(cond)) \
			sb_ebort("assertion failure !(%s)\n", #cond); \
	} while (0)
#define sb_fprintf(fp, ...) sb_fdprintf(fileno(fp), __VA_ARGS__)
#define sb_vfprintf(fp, ...) sb_vfdprintf(fileno(fp), __VA_ARGS__)

/* Memory functions */
void *__xcalloc(size_t nmemb, size_t size, const char *file, const char *func, size_t line);
void *__xmalloc(size_t size, const char *file, const char *func, size_t line);
void *__xzalloc(size_t size /*, const char *file, const char *func, size_t line */);
void *__xrealloc(void *ptr, size_t size, const char *file, const char *func, size_t line);
char *__xstrdup(const char *str, const char *file, const char *func, size_t line);
char *__xstrndup(const char *str, size_t size, const char *file, const char *func, size_t line);
#define xcalloc(_nmemb, _size) __xcalloc(_nmemb, _size, __FILE__, __func__, __LINE__)
#define xmalloc(_size)         __xmalloc(_size, __FILE__, __func__, __LINE__)
#define xzalloc(_size)         __xzalloc(_size /*, __FILE__, __func__, __LINE__ */)
#define xrealloc(_ptr, _size)  __xrealloc(_ptr, _size, __FILE__, __func__, __LINE__)
#define xstrdup(_str)          __xstrdup(_str, __FILE__, __func__, __LINE__)
#define xstrndup(_str, _size)  __xstrndup(_str, _size, __FILE__, __func__, __LINE__)
#define xalloc_die()           __sb_ebort(__FILE__, __func__, __LINE__, "out of memory")

#define xasprintf(fmt, ...) \
({ \
	int _ret = asprintf(fmt, __VA_ARGS__); \
	if (_ret == 0) \
		sb_perr("asprintf(%s) failed", #fmt); \
	_ret; \
})

/* string helpers */
#define streq(s1, s2) (strcmp(s1, s2) == 0)

/* errno helpers */
#define save_errno()    int old_errno = errno;
#define restore_errno() errno = old_errno;
#define saved_errno     old_errno

#endif /* __SBUTIL_H__ */
