/*
 * sandbox.c
 *
 * Main sandbox related functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 * Some parts might have Copyright:
 *
 *   Copyright (C) 2002 Brad House <brad@mainstreetsoftworks.com>
 */

#include "headers.h"
#include "sbutil.h"
#include "sandbox.h"

/* The C library might have a macro for this. */
#undef dprintf

static int print_debug = 0;
#define dprintf(fmt, args...) do { if (print_debug) printf(fmt, ## args); } while (0)
#define dputs(str) do { if (print_debug) puts(str); } while (0)
int (*sbio_faccessat)(int, const char *, int, int) = faccessat;
int (*sbio_open)(const char *, int, mode_t) = (void *)open;
FILE *(*sbio_popen)(const char *, const char *) = popen;

volatile static int stop_called = 0;
volatile static pid_t child_pid = 0;

static const char sandbox_banner[] = "============================= Gentoo path sandbox ==============================";
static const char sandbox_footer[] = "--------------------------------------------------------------------------------";
const char *sbio_message_path;
const char sbio_fallback_path[] = "/dev/stderr";

static int setup_sandbox(struct sandbox_info_t *sandbox_info, bool interactive)
{
	if (NULL != getenv(ENV_PORTAGE_TMPDIR)) {
		/* Portage handle setting SANDBOX_WRITE itself. */
		sandbox_info->work_dir[0] = '\0';
	} else {
		if (NULL == getcwd(sandbox_info->work_dir, SB_PATH_MAX)) {
			sb_pwarn("failed to get current directory");
			return -1;
		}
		if (interactive)
			setenv(ENV_SANDBOX_WORKDIR, sandbox_info->work_dir, 1);
	}

	if (-1 == get_tmp_dir(sandbox_info->tmp_dir)) {
		sb_pwarn("failed to get tmp_dir");
		return -1;
	}
	setenv(ENV_TMPDIR, sandbox_info->tmp_dir, 1);

	sandbox_info->home_dir = getenv("HOME");
	if (!sandbox_info->home_dir) {
		sandbox_info->home_dir = sandbox_info->tmp_dir;
		setenv("HOME", sandbox_info->home_dir, 1);
	}

	/* Generate sandbox lib path */
	get_sandbox_lib(sandbox_info->sandbox_lib);

	/* Generate sandbox bashrc path */
	get_sandbox_rc(sandbox_info->sandbox_rc);

	/* Generate sandbox log full path */
	get_sandbox_log(sandbox_info->sandbox_log, sandbox_info->tmp_dir);
	if (rc_file_exists(sandbox_info->sandbox_log)) {
		if (-1 == unlink(sandbox_info->sandbox_log)) {
			sb_pwarn("could not unlink old log file: %s",
			         sandbox_info->sandbox_log);
			return -1;
		}
	}

	/* Generate sandbox debug log full path */
	get_sandbox_debug_log(sandbox_info->sandbox_debug_log, sandbox_info->tmp_dir);
	if (rc_file_exists(sandbox_info->sandbox_debug_log)) {
		if (-1 == unlink(sandbox_info->sandbox_debug_log)) {
			sb_pwarn("could not unlink old debug log file: %s",
			         sandbox_info->sandbox_debug_log);
			return -1;
		}
	}

	/* Generate sandbox message path -- this process's stderr */
	const char *fdpath = sb_get_fd_dir();
	if (realpath(fdpath, sandbox_info->sandbox_message_path) == NULL) {
		sb_pwarn("could not read fd path: %s", fdpath);
		if (realpath(sbio_fallback_path, sandbox_info->sandbox_message_path)) {
			sb_pwarn("could not read stderr path: %s", sbio_fallback_path);
			/* fuck it */
			strcpy(sandbox_info->sandbox_message_path, sbio_fallback_path);
		}
	} else {
		/* Do not resolve the target of stderr because it could be something
		 * that doesn't exist on the fs.  Like a pipe (`tee` and such). */
		strcat(sandbox_info->sandbox_message_path, "/2");
	}

	return 0;
}

static void print_sandbox_log(char *sandbox_log)
{
	int sandbox_log_file;
	size_t len;
	char buffer[8192];

	sandbox_log_file = sb_open(sandbox_log, O_RDONLY, 0);
	if (-1 == sandbox_log_file) {
		sb_pwarn("could not open log file: %s", sandbox_log);
		return;
	}

	sb_eerror("----------------------- SANDBOX ACCESS VIOLATION SUMMARY -----------------------\n");
	sb_eerror("LOG FILE: \"%s\"\n", sandbox_log);

	while (1) {
		len = sb_read(sandbox_log_file, buffer, sizeof(buffer));
		if (len == -1) {
			sb_pwarn("sb_read(logfile) failed");
			break;
		} else if (!len)
			break;
		sb_eerror("\n%s", buffer);
	}
	sb_close(sandbox_log_file);

	sb_eerror("--------------------------------------------------------------------------------\n");
}

static int stop_count = 5;

static void stop(int signum)
{
	if (0 == stop_called) {
		stop_called = signum;
		sb_warn("caught signal %d in pid %d", signum, getpid());
	} else if (--stop_count) {
		sb_warn("Send signal %i more time%s to force SIGKILL",
			stop_count, stop_count == 1 ? "" : "s");
	} else {
		/* This really should kill all children; see usr1_handler. */
		kill(child_pid, SIGKILL);
		stop_count = 1;
	}
}

static void usr1_handler(int signum, siginfo_t *siginfo, void *ucontext)
{
	if (0 == stop_called) {
		stop_called = signum;
		sb_warn("caught signal %d in pid %d", signum, getpid());

		/* FIXME: This is really bad form, as we should kill the whole process
		 *        tree, but currently that is too much work and not worth the
		 *        effort.  Thus we only kill the calling process and our child
		 *        for now.
		 */
		if (siginfo->si_pid > 0)
			kill(siginfo->si_pid, SIGKILL);
		kill(child_pid, SIGKILL);
	} else
		sb_warn("signal already caught and busy still cleaning up!");
}

static int spawn_shell(char *argv_bash[], char **env, int debug)
{
	int status = 0;
	int ret = 0;

	child_pid = opt_use_namespaces ? setup_namespaces() : fork();

	/* Child's process */
	if (0 == child_pid) {
		/* Would be nice if execvpe were in POSIX. */
		environ = env;
		int ret = execvp(argv_bash[0], argv_bash);
		sb_pwarn("failed to exec child");
		_exit(ret);
	} else if (child_pid < 0) {
		if (debug)
			sb_pwarn("process failed to spawn!");
		return 1;
	}

	/* fork() creates a copy of this, so no need to use more memory than
	 * absolutely needed. */
	str_list_free(argv_bash);
	str_list_free(env);

	ret = waitpid(child_pid, &status, 0);
	if (-1 == ret) {
		sb_pwarn("failed to waitpid for child");
		return 1;
	} else if (status != 0) {
		if (WIFSIGNALED(status))
			psignal(WTERMSIG(status), "Sandboxed process killed by signal");
		else if (debug)
			sb_warn("process returned with failed exit status %d!", WEXITSTATUS(status));
		return status;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int sandbox_log_presence = 0;

	struct sandbox_info_t sandbox_info;

	char **sandbox_environ;
	char **argv_bash = NULL;

	char *run_str = "-c";

	/* Process the sandbox opts and leave argc/argv for the target. */
	parseargs(argc, argv);
	argc -= optind - 1;
	argv[optind - 1] = argv[0];
	argv += optind - 1;
	/* Only print info if called with no arguments .... */
	if (argc < 2)
		print_debug = 1;

	dputs(sandbox_banner);

	/* check if a sandbox is already running */
	if (!is_env_on(ENV_SANDBOX_TESTING))
		if (NULL != getenv(ENV_SANDBOX_ACTIVE))
			sb_err("not launching a new sandbox as one is already running in this process hierarchy");

	/* determine the location of all the sandbox support files */
	if (opt_debug)
		dputs("Detection of the support files.");

	if (-1 == setup_sandbox(&sandbox_info, print_debug))
		sb_err("failed to setup sandbox");

	/* verify the existance of required files */
	if (opt_debug)
		dputs("Verification of the required files.");

	if (!rc_file_exists(sandbox_info.sandbox_rc))
		sb_perr("could not open the sandbox rc file: %s", sandbox_info.sandbox_rc);

	/* set up the required environment variables */
	if (opt_debug)
		dputs("Setting up the required environment variables.");

	/* If not in portage, cd into it work directory */
	if ('\0' != sandbox_info.work_dir[0])
		if (chdir(sandbox_info.work_dir))
			sb_perr("chdir(%s) failed", sandbox_info.work_dir);

	/* Setup the child environment stuff.
	 * XXX:  We free this in spawn_shell(). */
	sandbox_environ = setup_environ(&sandbox_info, print_debug);
	if (NULL == sandbox_environ)
		goto oom_error;

	/* Setup bash argv */
	if (!opt_use_bash && argc == 2) {
		/* Backwards compatibility hack: if there's only one argument, and it
		 * appears to be a shell command (not an absolute path to a program),
		 * then fallback to running through the shell.
		 */
		if (access(argv[1], X_OK))
			opt_use_bash = true;
	}

	if (opt_use_bash || argc == 1) {
		str_list_add_item_copy(argv_bash, "/bin/bash", oom_error);
		str_list_add_item_copy(argv_bash, "-rcfile", oom_error);
		str_list_add_item_copy(argv_bash, sandbox_info.sandbox_rc, oom_error);
		if (argc >= 2) {
			int i;
			size_t cmdlen;

			str_list_add_item_copy(argv_bash, run_str, oom_error);
			str_list_add_item_copy(argv_bash, argv[1], oom_error);
			cmdlen = strlen(argv_bash[4]);
			for (i = 2; i < argc; i++) {
				size_t arglen = strlen(argv[i]);
				argv_bash[4] = xrealloc(argv_bash[4], cmdlen + arglen + 2);
				argv_bash[4][cmdlen] = ' ';
				memcpy(argv_bash[4] + cmdlen + 1, argv[i], arglen);
				cmdlen += arglen + 1;
				argv_bash[4][cmdlen] = '\0';
			}
		}
	} else {
		int i;
		for (i = 1; i < argc; ++i)
			str_list_add_item_copy(argv_bash, argv[i], oom_error);
	}

#ifdef HAVE_PRCTL
	/* Lock down access to elevated privileges.  In practice, this will block
	 * use of tools like su and sudo, and will allow use of seccomp bpf.
	 */
# ifdef PR_SET_NO_NEW_PRIVS
	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1) {
		/* Ignore EINVAL in case we're on old kernels.  Unfortunately we can't
		 * differentiate between EINVAL due to unsupported PR_xxx and EINVAL
		 * due to bad 2nd/3rd/4th/5th args.
		 */
		if (errno != EINVAL)
			sb_eerror("prctl(PR_SET_NO_NEW_PRIVS) failed");
	}
# endif
#endif

	/* Set up the required signal handlers */
	int sigs[] = { SIGHUP, SIGINT, SIGQUIT, SIGTERM, SIGUSR1, };
	struct sigaction act_new, act_old[ARRAY_SIZE(sigs)];
	size_t si = 0;

#define wsigaction() \
	do { \
		if (sigaction(sigs[si], &act_new, &act_old[si])) \
			sb_pwarn("unable to bind signal %i", sigs[si]); \
		else if (act_old[si].sa_handler != SIG_DFL && \
		         act_old[si].sa_handler != SIG_IGN) \
			sb_warn("signal %i already had a handler ...", sigs[si]); \
		++si; \
	} while (0)

	sigemptyset(&act_new.sa_mask);
	act_new.sa_sigaction = NULL;
	act_new.sa_handler = stop;
	act_new.sa_flags = SA_RESTART;
	wsigaction();
	wsigaction();
	wsigaction();
	wsigaction();

	sigemptyset(&act_new.sa_mask);
	act_new.sa_handler = NULL;
	act_new.sa_sigaction = usr1_handler;
	act_new.sa_flags = SA_SIGINFO | SA_RESTART;
	wsigaction();

	/* Allow SIGHUP to be ignored in case people are running `nohup ...` #217898 */
	if (act_old[0].sa_handler == SIG_IGN)
		sigaction(SIGHUP, &act_old[0], NULL);

	/* STARTING PROTECTED ENVIRONMENT */
	if (opt_debug)
		dputs("The protected environment has been started.");

	/* Start Bash */
	int shell_exit = spawn_shell(argv_bash, sandbox_environ, print_debug);

	/* As spawn_shell() free both argv_bash and sandbox_environ, make sure
	 * we do not run into issues in future if we need a OOM error below
	 * this ... */
	argv_bash = NULL;
	sandbox_environ = NULL;

	dputs("The protected environment has been shut down.");

	if (rc_file_exists(sandbox_info.sandbox_log)) {
		sandbox_log_presence = 1;
		print_sandbox_log(sandbox_info.sandbox_log);
	} else
		dputs(sandbox_footer);

	/* Do the right thing and pass the signal back up.  See:
	 * https://www.cons.org/cracauer/sigint.html
	 */
	if (stop_called != SIGUSR1 && WIFSIGNALED(shell_exit)) {
		int signum = WTERMSIG(shell_exit);
		for (si = 0; si < ARRAY_SIZE(sigs); ++si)
			sigaction(sigs[si], &act_old[si], NULL);
		kill(getpid(), signum);
		return 128 + signum;
	} else if (WIFEXITED(shell_exit))
		shell_exit = WEXITSTATUS(shell_exit);
	else
		shell_exit = 1; /* ??? */

	if (!is_env_on(ENV_SANDBOX_TESTING))
		if (sandbox_log_presence && shell_exit == 0)
			shell_exit = 1;
	return shell_exit;

oom_error:
	if (NULL != argv_bash)
		str_list_free(argv_bash);

	sb_perr("out of memory (environ)");
}
