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

#define LD_PRELOAD_FILE        "/etc/ld.so.preload"
#define LIB_NAME               "libsandbox.so"
#define BASHRC_NAME            "sandbox.bashrc"
#define TMPDIR                 "/tmp"
#define VAR_TMPDIR             "/var/tmp"
#define PORTAGE_TMPDIR         "/var/tmp/portage"
#define PIDS_FILE              "/sandboxpids.tmp"
#define LOG_FILE_PREFIX        "/sandbox-"
#define DEBUG_LOG_FILE_PREFIX  "/sandbox-debug-"
#define LOG_FILE_EXT           ".log"

#define ENV_TMPDIR             "TMPDIR"
#define ENV_PORTAGE_TMPDIR     "PORTAGE_TMPDIR"

#define ENV_SANDBOX_DEBUG_LOG  "SANDBOX_DEBUG_LOG"
#define ENV_SANDBOX_LOG        "SANDBOX_LOG"
#define ENV_SANDBOX_DIR        "SANDBOX_DIR"
#define ENV_SANDBOX_LIB        "SANDBOX_LIB"
#define ENV_SANDBOX_BASHRC     "SANDBOX_BASHRC"

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

SB_STATIC char *get_sandbox_path(char *argv0);
SB_STATIC char *get_sandbox_lib(char *sb_path);
SB_STATIC char *get_sandbox_pids_file(const char *tmp_dir);
SB_STATIC char *get_sandbox_rc(char *sb_path);
SB_STATIC char *get_sandbox_log(const char *tmp_dir);
SB_STATIC char *get_sandbox_debug_log(const char *tmp_dir);
SB_STATIC char *sb_dirname(const char *path);
SB_STATIC int file_getmode(char *mode);
SB_STATIC long file_tell(int fp);
SB_STATIC int file_lock(int fd, int lock, char *filename);
SB_STATIC int file_unlock(int fd);
SB_STATIC int file_locktype(char *mode);
SB_STATIC int file_open(char *filename, char *mode, int perm_specified, ...);
SB_STATIC void file_close(int fd);
SB_STATIC long file_length(int fd);
SB_STATIC int file_truncate(int fd);
SB_STATIC int file_exist(char *filename, int checkmode);

#endif

// vim:noexpandtab noai:cindent ai
