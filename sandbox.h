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

SB_STATIC void get_sandbox_lib(char *path);
SB_STATIC void get_sandbox_rc(char *path);
SB_STATIC void get_sandbox_log(char *path);
SB_STATIC void get_sandbox_debug_log(char *path);
SB_STATIC int get_tmp_dir(char *path);
SB_STATIC int exists(const char *pathname);
SB_STATIC int is_file(const char *pathname);
SB_STATIC long file_length(int fd);

#endif

// vim:noexpandtab noai:cindent ai
