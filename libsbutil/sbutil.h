/*
 * sbutil.h
 *
 * Util defines.
 *
 * Copyright 1999-2006 Gentoo Foundation
 *
 *
 *      This program is free software; you can redistribute it and/or modify it
 *      under the terms of the GNU General Public License as published by the
 *      Free Software Foundation version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Some parts might have Copyright:
 *
 *   Copyright (C) 2002 Brad House <brad@mainstreetsoftworks.com>
 *
 * $Header$
 */

#ifndef __SBUTIL_H__
#define __SBUTIL_H__

#include "config.h"
#include "localdecls.h"
#include "include/rcscripts/rcutil.h"

#define SANDBOX_CONF_FILE      ETCDIR "/sandbox.conf"
#define SANDBOX_CONFD_DIR      ETCDIR "/sandbox.d"

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
void get_sandbox_rc(char *path);
void get_sandbox_log(char *path);
void get_sandbox_debug_log(char *path);
int get_tmp_dir(char *path);
bool is_env_on (const char *);
bool is_env_off (const char *);

/* glibc modified realpath() function */
char *erealpath(const char *, char *);
char *egetcwd(char *, size_t);

#endif /* __SBUTIL_H__ */
