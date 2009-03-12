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

static int print_debug = 0;
#define dprintf(fmt, args...) do { if (print_debug) printf(fmt, ## args); } while (0)
#define dputs(str) do { if (print_debug) puts(str); } while (0)

volatile static int stop_called = 0;
volatile static pid_t child_pid = 0;

static char log_domain[] = "sandbox";
static const char sandbox_banner[] = "============================= Gentoo path sandbox ==============================";
static const char sandbox_footer[] = "--------------------------------------------------------------------------------";

const char env_sandbox_testing[] = "__SANDBOX_TESTING";

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
	get_sandbox_log(sandbox_info->sandbox_log);
	if (rc_file_exists(sandbox_info->sandbox_log)) {
		if (-1 == unlink(sandbox_info->sandbox_log)) {
			sb_pwarn("could not unlink old log file: %s",
			         sandbox_info->sandbox_log);
			return -1;
		}
	}

	/* Generate sandbox debug log full path */
	get_sandbox_debug_log(sandbox_info->sandbox_debug_log);
	if (rc_file_exists(sandbox_info->sandbox_debug_log)) {
		if (-1 == unlink(sandbox_info->sandbox_debug_log)) {
			sb_pwarn("could not unlink old debug log file: %s",
			         sandbox_info->sandbox_debug_log);
			return -1;
		}
	}

	return 0;
}

static void print_sandbox_log(char *sandbox_log)
{
	int sandbox_log_file, beep_count;
	char *beep_count_env;
	size_t len;
	char buffer[1024];

	sandbox_log_file = sb_open(sandbox_log, O_RDONLY, 0);
	if (-1 == sandbox_log_file) {
		sb_pwarn("could not open log file: %s", sandbox_log);
		return;
	}

	SB_EERROR("--------------------------- ACCESS VIOLATION SUMMARY ---------------------------", "\n");
	SB_EERROR("LOG FILE", " \"%s\"\n\n", sandbox_log);

	while (1) {
		len = sb_read(sandbox_log_file, buffer, sizeof(buffer));
		if (len == -1) {
			sb_pwarn("sb_read(logfile) failed");
			break;
		} else if (!len)
			break;
		if (sb_write(STDERR_FILENO, buffer, len) != len) {
			sb_pwarn("sb_write(logfile) failed");
			break;
		}
	}
	sb_close(sandbox_log_file);

	SB_EERROR("--------------------------------------------------------------------------------", "\n");

	beep_count_env = getenv(ENV_SANDBOX_BEEP);
	if (beep_count_env)
		beep_count = atoi(beep_count_env);
	else
		beep_count = DEFAULT_BEEP_COUNT;

	while (beep_count--) {
		fputc('\a', stderr);
		if (beep_count)
			sleep(1);
	}
}

static void stop(int signum)
{
	if (0 == stop_called) {
		stop_called = 1;
		sb_warn("caught signal %d in pid %d\n", signum, getpid());
	} else
		sb_warn("signal already caught and busy still cleaning up!");
}

static void usr1_handler(int signum, siginfo_t *siginfo, void *ucontext)
{
	if (0 == stop_called) {
		stop_called = 1;
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

	child_pid = fork();

	/* Child's process */
	if (0 == child_pid) {
		int ret = execve(argv_bash[0], argv_bash, env);
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
		if (WIFSIGNALED(status)) {
			psignal(WTERMSIG(status), "Sandboxed process killed by signal");
			return 128 + WTERMSIG(status);
		} else if (debug)
			sb_warn("process returned with failed exit status %d!", WEXITSTATUS(status));
		return WEXITSTATUS(status) ? : 1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct sigaction act_new;

	int sandbox_log_presence = 0;

	struct sandbox_info_t sandbox_info;

	char **sandbox_environ;
	char **argv_bash = NULL;

	char *run_str = "-c";

	rc_log_domain(log_domain);

	/* Only print info if called with no arguments .... */
	if (argc < 2)
		print_debug = 1;
	else {
		/* handle a few common options */
		if (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-V")) {
			puts(
				"Gentoo path sandbox\n"
				" version: " PACKAGE_VERSION "\n"
				" C lib:   " LIBC_VERSION " (" LIBC_PATH ")\n"
				" build:   " __DATE__ " " __TIME__ "\n"
				" contact: " PACKAGE_BUGREPORT " via http://bugs.gentoo.org/\n"
				" rtld:    "
#ifdef BROKEN_RTLD_NEXT
					"next is broken ;(\n"
#else
					"next is OK! :D\n"
#endif
				"\nconfigured with these options:\n"
				SANDBOX_CONFIGURE_OPTS
			);
			return 0;
		} else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
			puts(
				"Usage: sandbox [program [program args...]]\n"
				"\n"
				"Sandbox will start up a sandbox session and execute the specified program.\n"
				"If no program is specified, an interactive shell is automatically launched.\n"
				"You can use this to quickly test out sandbox behavior.\n"
				"\n"
				"Upon startup, initial settings are taken from these files / directories:\n"
				"\t" SANDBOX_CONF_FILE "\n"
				"\t" SANDBOX_CONFD_DIR "\n"
				"\n"
				"Contact: " PACKAGE_BUGREPORT " via http://bugs.gentoo.org/"
			);
			return 0;
		}
	}

	dputs(sandbox_banner);

	/* check if a sandbox is already running */
	if (!is_env_on(ENV_SANDBOX_TESTING))
		if (NULL != getenv(ENV_SANDBOX_ACTIVE))
			sb_err("not launching a new sandbox as one is already running in this process hierarchy");

	/* determine the location of all the sandbox support files */
	dputs("Detection of the support files.");

	if (-1 == setup_sandbox(&sandbox_info, print_debug))
		sb_err("failed to setup sandbox");

	/* verify the existance of required files */
	dputs("Verification of the required files.");

	if (!rc_file_exists(sandbox_info.sandbox_rc))
		sb_perr("could not open the sandbox rc file: %s", sandbox_info.sandbox_rc);

	/* set up the required environment variables */
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
	str_list_add_item_copy(argv_bash, "/bin/bash", oom_error);
	str_list_add_item_copy(argv_bash, "-rcfile", oom_error);
	str_list_add_item_copy(argv_bash, sandbox_info.sandbox_rc, oom_error);
	if (argc >= 2) {
		int i;

		str_list_add_item_copy(argv_bash, run_str, oom_error);
		str_list_add_item_copy(argv_bash, argv[1], oom_error);
		for (i = 2; i < argc; i++) {
			char *tmp_ptr;

			tmp_ptr = xrealloc(argv_bash[4],
					   (strlen(argv_bash[4]) +
					    strlen(argv[i]) + 2) *
					   sizeof(char));
			argv_bash[4] = tmp_ptr;

			snprintf(argv_bash[4] + strlen(argv_bash[4]),
				 strlen(argv[i]) + 2, " %s",
				 argv[i]);
		}
	}

	/* set up the required signal handlers ... but allow SIGHUP to be
	 * ignored in case people are running `nohup ...` #217898
	 */
	if (signal(SIGHUP, &stop) == SIG_IGN)
		signal(SIGHUP, SIG_IGN);
#define wsignal(sig, act) \
	do { \
		sighandler_t _old = signal(sig, act); \
		if (_old == SIG_ERR) \
			sb_pwarn("unable to bind signal %s\n", #sig); \
		else if (_old != SIG_DFL) \
			sb_warn("signal %s already had a handler ...\n", #sig); \
	} while (0)
	wsignal(SIGINT, &stop);
	wsignal(SIGQUIT, &stop);
	wsignal(SIGTERM, &stop);
	act_new.sa_sigaction = usr1_handler;
	sigemptyset (&act_new.sa_mask);
	act_new.sa_flags = SA_SIGINFO | SA_RESTART;
	sigaction (SIGUSR1, &act_new, NULL);

	/* STARTING PROTECTED ENVIRONMENT */
	dputs("The protected environment has been started.");
	dputs(sandbox_footer);
	dputs("Process being started in forked instance.");

	/* Start Bash */
	int shell_exit = spawn_shell(argv_bash, sandbox_environ, print_debug);

	/* As spawn_shell() free both argv_bash and sandbox_environ, make sure
	 * we do not run into issues in future if we need a OOM error below
	 * this ... */
	argv_bash = NULL;
	sandbox_environ = NULL;

	dputs("Cleaning up sandbox process");
	dputs(sandbox_banner);
	dputs("The protected environment has been shut down.");

	if (rc_file_exists(sandbox_info.sandbox_log)) {
		sandbox_log_presence = 1;
		print_sandbox_log(sandbox_info.sandbox_log);
	} else
		dputs(sandbox_footer);

	if (sandbox_log_presence && shell_exit == 0)
		shell_exit = 1;
	return shell_exit;

oom_error:
	if (NULL != argv_bash)
		str_list_free(argv_bash);

	sb_perr("out of memory (environ)");
}
