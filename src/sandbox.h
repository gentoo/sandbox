/*
 * sandbox.h
 *
 * Main sandbox related functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 * Some parts might have Copyright:
 *   Copyright (C) 2002 Brad House <brad@mainstreetsoftworks.com>
 */

#ifndef __SANDBOX_H__
#define __SANDBOX_H__

struct sandbox_info_t {
	char sandbox_log[PATH_MAX];
	char sandbox_debug_log[PATH_MAX];
	char sandbox_message_path[PATH_MAX];
	char sandbox_lib[PATH_MAX];
	char sandbox_rc[PATH_MAX];
	char work_dir[PATH_MAX];
	char tmp_dir[PATH_MAX];
	char *home_dir;
};

extern char **setup_environ(struct sandbox_info_t *sandbox_info);

extern bool sb_get_cnf_bool(const char *, bool);

#ifdef __linux__
extern pid_t setup_namespaces(void);
#else
#define setup_namespaces() fork()
#endif

#define sb_warn(fmt, args...)  fprintf(stderr, "%s:%s  " fmt "\n", "sandbox", __func__, ## args)
#define sb_pwarn(fmt, args...) sb_warn(fmt ": %s\n", ## args, strerror(errno))
#define _sb_err(func, fmt, args...) do { sb_##func(fmt, ## args); exit(EXIT_FAILURE); } while (0)
#define sb_err(fmt, args...)  _sb_err(warn, fmt, ## args)
#define sb_perr(fmt, args...) _sb_err(pwarn, fmt, ## args)

/* Option parsing related code */
extern void parseargs(int argc, char *argv[]);
extern int opt_use_namespaces;
extern int opt_use_ns_cgroup;
extern int opt_use_ns_ipc;
extern int opt_use_ns_mnt;
extern int opt_use_ns_net;
extern int opt_use_ns_pid;
extern int opt_use_ns_sysv;
extern int opt_use_ns_time;
extern int opt_use_ns_user;
extern int opt_use_ns_uts;
extern bool opt_use_bash;

#endif
