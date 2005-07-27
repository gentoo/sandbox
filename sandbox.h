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

/* Uncomment below to use flock instead of fcntl (POSIX way) to lock/unlock files */
/* #define USE_FLOCK */

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

#define ENV_TMPDIR             "TMPDIR"
#define ENV_PORTAGE_TMPDIR     "PORTAGE_TMPDIR"

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

#define DEFAULT_BEEP_COUNT     3

#define SB_BUF_LEN             2048

#if !HAVE_DLVSYM
# define dlvsym(lib, sym, ver) dlsym(lib, sym)
#endif

/* Gentoo style e* printing macro's */
#define EINFO(_color, _hilight, _args...) \
	do { \
		int old_errno = errno; \
		if (_color) \
			fprintf(stderr, "\033[32;01m" _hilight "\033[0m" _args); \
		else \
			fprintf(stderr, _hilight _args); \
		errno = old_errno; \
	} while (0)

#define EWARN(_color, _hilight, _args...) \
	do { \
		int old_errno = errno; \
		if (_color) \
			fprintf(stderr, "\033[33;01m" _hilight "\033[0m" _args); \
		else \
			fprintf(stderr, _hilight _args); \
		errno = old_errno; \
	} while (0)

#define EERROR(_color, _hilight, _args...) \
	do { \
		int old_errno = errno; \
		if (_color) \
			fprintf(stderr, "\033[31;01m" _hilight "\033[0m" _args); \
		else \
			fprintf(stderr, _hilight _args); \
		errno = old_errno; \
	} while (0)

SB_STATIC void get_sandbox_lib(char *path);
#ifdef OUTSIDE_LIBSANDBOX
SB_STATIC void get_sandbox_rc(char *path);
SB_STATIC void get_sandbox_log(char *path);
SB_STATIC void get_sandbox_debug_log(char *path);
SB_STATIC int get_tmp_dir(char *path);
#endif /* OUTSIDE_LIBSANDBOX */
SB_STATIC int exists(const char *pathname);
#ifdef OUTSIDE_LIBSANDBOX
SB_STATIC int is_file(const char *pathname);
SB_STATIC int is_dir(const char *pathname, int follow_link);
SB_STATIC long file_length(int fd);
#endif /* OUTSIDE_LIBSANDBOX */

#endif

// vim:noexpandtab noai:cindent ai
