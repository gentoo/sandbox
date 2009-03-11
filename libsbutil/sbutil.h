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

#define LD_PRELOAD_EQ          "LD_PRELOAD="
#define LD_PRELOAD_FILE        "/etc/ld.so.preload"
#define LIB_NAME               "libsandbox.so"
#define BASHRC_NAME            "sandbox.bashrc"
#define TMPDIR                 "/tmp"
#define PORTAGE_TMPDIR         "/var/tmp/portage"
#define SANDBOX_LOG_LOCATION   "/var/log/sandbox"
#define LOG_FILE_PREFIX        "/sandbox-"
#define DEBUG_LOG_FILE_PREFIX  "/sandbox-debug-"
#define LOG_FILE_EXT           ".log"

#define ENV_LD_PRELOAD         "LD_PRELOAD"

#define ENV_EBUILD             "EBUILD"
#define ENV_TMPDIR             "TMPDIR"
#define ENV_PORTAGE_TMPDIR     "PORTAGE_TMPDIR"

#define ENV_BASH_ENV           "BASH_ENV"

#define ENV_NOCOLOR            "NOCOLOR"

#define ENV_SANDBOX_VERBOSE    "SANDBOX_VERBOSE"
#define ENV_SANDBOX_DEBUG      "SANDBOX_DEBUG"

extern const char env_sandbox_testing[];
#define ENV_SANDBOX_TESTING    env_sandbox_testing

#define ENV_SANDBOX_LIB        "SANDBOX_LIB"
#define ENV_SANDBOX_BASHRC     "SANDBOX_BASHRC"
#define ENV_SANDBOX_LOG        "SANDBOX_LOG"
#define ENV_SANDBOX_DEBUG_LOG  "SANDBOX_DEBUG_LOG"
#define ENV_SANDBOX_WORKDIR    "SANDBOX_WORKDIR"

#define ENV_SANDBOX_DENY       "SANDBOX_DENY"
#define ENV_SANDBOX_READ       "SANDBOX_READ"
#define ENV_SANDBOX_WRITE      "SANDBOX_WRITE"
#define ENV_SANDBOX_PREDICT    "SANDBOX_PREDICT"

#define ENV_SANDBOX_ON         "SANDBOX_ON"
#define ENV_SANDBOX_BEEP       "SANDBOX_BEEP"

#define ENV_SANDBOX_PID        "SANDBOX_PID"
#define ENV_SANDBOX_ABORT      "SANDBOX_ABORT"
#define ENV_SANDBOX_INTRACTV   "SANDBOX_INTRACTV"

#define ENV_SANDBOX_ACTIVE     "SANDBOX_ACTIVE"
#define SANDBOX_ACTIVE         "armedandready"

#define DEFAULT_BEEP_COUNT     3

#define SB_BUF_LEN             2048

#define COLOR_NORMAL           "\033[0m"
#define COLOR_GREEN            "\033[32;01m"
#define COLOR_YELLOW           "\033[33;01m"
#define COLOR_RED              "\033[31;01m"

/* Gentoo style e* printing macro's */
#define SB_EINFO(_hilight, _args...) sb_efunc(COLOR_GREEN, _hilight, _args)
#define SB_EWARN(_hilight, _args...) sb_efunc(COLOR_YELLOW, _hilight, _args)
#define SB_EERROR(_hilight, _args...) sb_efunc(COLOR_RED, _hilight, _args)

char *get_sandbox_conf(void);
void get_sandbox_lib(char *path);
void get_sandbox_rc(char *path);
void get_sandbox_log(char *path);
void get_sandbox_debug_log(char *path);
int get_tmp_dir(char *path);
bool is_env_on (const char *);
bool is_env_off (const char *);

/* libsandbox need to use a wrapper for open */
void sb_set_open(void *new_open);
/* Convenience functions to reliably open, read and write to a file */
int sb_open(const char *path, int flags, mode_t mode);
size_t sb_read(int fd, void *buf, size_t count);
size_t sb_write(int fd, const void *buf, size_t count);
int sb_close(int fd);

/* Reliable output */
__attribute__((__format__(__printf__, 1, 2))) void sb_printf(const char *format, ...);
__attribute__((__format__(__printf__, 2, 3))) void sb_fdprintf(int fd, const char *format, ...);
__attribute__((__format__(__printf__, 2, 0))) void sb_vfdprintf(int fd, const char *format, va_list args);
__attribute__((__format__(__printf__, 3, 4))) void sb_efunc(const char *color, const char *hilight, const char *format, ...);
#define sb_fprintf(fp, ...) sb_fdprintf(fileno(fp), __VA_ARGS__)

/* Macro for sb_read() to goto an label on error */
#define SB_WRITE(_fd, _buf, _count, _error) \
	do { \
		size_t _n; \
		_n = sb_write(_fd, _buf, _count); \
		if (-1 == _n) \
			goto _error; \
	} while (0)

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

/* errno helpers */
#define save_errno()    int old_errno = errno;
#define restore_errno() errno = old_errno;
#define saved_errno     old_errno

#endif /* __SBUTIL_H__ */
