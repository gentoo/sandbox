/* 
 * Copyright (C) 2002 Brad House <brad@mainstreetsoftworks.com>,
 * Possibly based on code from Geert Bevin, Uwyn, http://www.uwyn.com
 * Distributed under the terms of the GNU General Public License, v2 or later 
 * Author: Brad House <brad@mainstreetsoftworks.com>
 *    
 * $Header$
 */

#ifndef __SANDBOX_H__
#define __SANDBOX_H__

#include "localdecls.h"
#include "config.h"
#include "rcscripts/rctypes.h"

#define LD_PRELOAD_EQ          "LD_PRELOAD="
#define LD_PRELOAD_FILE        "/etc/ld.so.preload"
#define LIB_NAME               "libsandbox.so"
#define BASHRC_NAME            "sandbox.bashrc"
#define TMPDIR                 "/tmp"
#define VAR_TMPDIR             "/var/tmp"
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

#define ENV_SANDBOX_LIB        "SANDBOX_LIB"
#define ENV_SANDBOX_BASHRC     "SANDBOX_BASHRC"
#define ENV_SANDBOX_LOG        "SANDBOX_LOG"
#define ENV_SANDBOX_DEBUG_LOG  "SANDBOX_DEBUG_LOG"

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

/* Gentoo style e* printing macro's */
#define SB_EINFO(_color, _hilight, _args...) \
	do { \
		int old_errno = errno; \
		if (_color) \
			fprintf(stderr, "\033[32;01m" _hilight "\033[0m" _args); \
		else \
			fprintf(stderr, _hilight _args); \
		errno = old_errno; \
	} while (0)

#define SB_EWARN(_color, _hilight, _args...) \
	do { \
		int old_errno = errno; \
		if (_color) \
			fprintf(stderr, "\033[33;01m" _hilight "\033[0m" _args); \
		else \
			fprintf(stderr, _hilight _args); \
		errno = old_errno; \
	} while (0)

#define SB_EERROR(_color, _hilight, _args...) \
	do { \
		int old_errno = errno; \
		if (_color) \
			fprintf(stderr, "\033[31;01m" _hilight "\033[0m" _args); \
		else \
			fprintf(stderr, _hilight _args); \
		errno = old_errno; \
	} while (0)

void get_sandbox_lib(char *path);
#ifdef OUTSIDE_LIBSANDBOX
void get_sandbox_rc(char *path);
void get_sandbox_log(char *path);
void get_sandbox_debug_log(char *path);
int get_tmp_dir(char *path);
long file_length(int);
#endif /* OUTSIDE_LIBSANDBOX */
bool is_env_on (const char *);
bool is_env_off (const char *);

/* glibc modified realpath() function */
char *erealpath(const char *, char *);
#ifndef OUTSIDE_LIBSANDBOX
char *egetcwd(char *, size_t);
#endif /* !OUTSIDE_LIBSANDBOX */

#endif /* __SANDBOX_H__ */

// vim:noexpandtab noai:cindent ai
