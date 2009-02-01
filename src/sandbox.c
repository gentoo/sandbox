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

volatile static int stop_called = 0;
volatile static pid_t child_pid = 0;

static char log_domain[] = "sandbox";
static const char sandbox_banner[] = "============================= Gentoo path sandbox ==============================";

static int setup_sandbox(struct sandbox_info_t *sandbox_info, bool interactive)
{
	if (NULL != getenv(ENV_PORTAGE_TMPDIR)) {
		/* Portage handle setting SANDBOX_WRITE itself. */
		sandbox_info->work_dir[0] = '\0';
	} else {
		if (NULL == getcwd(sandbox_info->work_dir, SB_PATH_MAX)) {
			perror("sandbox:  Failed to get current directory");
			return -1;
		}
		if (interactive)
			setenv(ENV_SANDBOX_WORKDIR, sandbox_info->work_dir, 1);
	}

	/* Do not resolve symlinks, etc .. libsandbox will handle that. */
	if (!rc_is_dir(VAR_TMPDIR, true)) {
		perror("sandbox:  Failed to get var_tmp_dir");
		return -1;
	}
	snprintf(sandbox_info->var_tmp_dir, SB_PATH_MAX, "%s", VAR_TMPDIR);

	if (-1 == get_tmp_dir(sandbox_info->tmp_dir)) {
		perror("sandbox:  Failed to get tmp_dir");
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
			perror("sandbox:  Could not unlink old log file");
			return -1;
		}
	}

	/* Generate sandbox debug log full path */
	get_sandbox_debug_log(sandbox_info->sandbox_debug_log);
	if (rc_file_exists(sandbox_info->sandbox_debug_log)) {
		if (-1 == unlink(sandbox_info->sandbox_debug_log)) {
			perror("sandbox:  Could not unlink old debug log file");
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
		perror("sandbox:  Could not open Log file");
		return;
	}

	SB_EERROR("--------------------------- ACCESS VIOLATION SUMMARY ---------------------------", "\n");
	SB_EERROR("LOG FILE", " \"%s\"\n\n", sandbox_log);

	while (1) {
		len = sb_read(sandbox_log_file, buffer, sizeof(buffer));
		if (len == -1) {
			perror("sandbox:  sb_read(logfile) failed");
			break;
		} else if (!len)
			break;
		if (sb_write(STDERR_FILENO, buffer, len) != len) {
			perror("sandbox:  sb_write(logfile) failed");
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
		printf("sandbox:  Caught signal %d in pid %d\n",
		       signum, getpid());
	} else {
		fprintf(stderr,
			"sandbox:  Signal already caught and busy still cleaning up!\n");
	}
}

static void usr1_handler(int signum, siginfo_t *siginfo, void *ucontext)
{
	if (0 == stop_called) {
		stop_called = 1;
		printf("sandbox:  Caught signal %d in pid %d\n",
		       signum, getpid());

		/* FIXME: This is really bad form, as we should kill the whole process
		 *        tree, but currently that is too much work and not worth the
		 *        effort.  Thus we only kill the calling process and our child
		 *        for now.
		 */
		if (siginfo->si_pid > 0)
			kill(siginfo->si_pid, SIGKILL);
		kill(child_pid, SIGKILL);
	} else {
		fprintf(stderr,
			"sandbox:  Signal already caught and busy still cleaning up!\n");
	}
}

static int spawn_shell(char *argv_bash[], char **env, int debug)
{
	int status = 0;
	int ret = 0;

	child_pid = fork();

	/* Child's process */
	if (0 == child_pid) {
		int ret = execve(argv_bash[0], argv_bash, env);
		perror("sandbox:  Failed to exec child");
		_exit(ret);
	} else if (child_pid < 0) {
		if (debug)
			fprintf(stderr, "Process failed to spawn!\n");
		return 0;
	}

	/* fork() creates a copy of this, so no need to use more memory than
	 * absolutely needed. */
	str_list_free(argv_bash);
	str_list_free(env);

	ret = waitpid(child_pid, &status, 0);
	if (-1 == ret) {
		perror("sandbox:  Failed to waitpid for child");
		return 0;
	} else if (status != 0) {
		if (WIFSIGNALED(status))
			psignal(WTERMSIG(status), "Sandboxed process killed by signal");
		else if (debug)
			fprintf(stderr, "Process returned with failed exit status %d!\n", WEXITSTATUS(status));
		return 0;
	}

	return 1;
}

int main(int argc, char **argv)
{
	struct sigaction act_new;

	int success = 1;
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

	if (print_debug)
		puts(sandbox_banner);

	/* check if a sandbox is already running */
	if (NULL != getenv(ENV_SANDBOX_ACTIVE)) {
		fprintf(stderr, "Not launching a new sandbox instance\n");
		fprintf(stderr, "Another one is already running in this process hierarchy.\n");
		exit(EXIT_FAILURE);
	}

	/* determine the location of all the sandbox support files */
	if (print_debug)
		printf("Detection of the support files.\n");

	if (-1 == setup_sandbox(&sandbox_info, print_debug)) {
		fprintf(stderr, "sandbox:  Failed to setup sandbox.");
		exit(EXIT_FAILURE);
	}

	/* verify the existance of required files */
	if (print_debug)
		printf("Verification of the required files.\n");

	if (!rc_file_exists(sandbox_info.sandbox_rc)) {
		perror("sandbox:  Could not open the sandbox rc file");
		exit(EXIT_FAILURE);
	}

	/* set up the required environment variables */
	if (print_debug)
		printf("Setting up the required environment variables.\n");

	/* If not in portage, cd into it work directory */
	if ('\0' != sandbox_info.work_dir[0])
		chdir(sandbox_info.work_dir);

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
			if (NULL == tmp_ptr)
				goto oom_error;
			argv_bash[4] = tmp_ptr;

			snprintf(argv_bash[4] + strlen(argv_bash[4]),
				 strlen(argv[i]) + 2, " %s",
				 argv[i]);
		}
	}

	/* set up the required signal handlers */
	signal(SIGHUP, &stop);
	signal(SIGINT, &stop);
	signal(SIGQUIT, &stop);
	signal(SIGTERM, &stop);
	act_new.sa_sigaction = usr1_handler;
	sigemptyset (&act_new.sa_mask);
	act_new.sa_flags = SA_SIGINFO | SA_RESTART;
	sigaction (SIGUSR1, &act_new, NULL);

	/* STARTING PROTECTED ENVIRONMENT */
	if (print_debug) {
		printf("The protected environment has been started.\n");
		printf("--------------------------------------------------------------------------------\n");
	}

	if (print_debug)
		printf("Process being started in forked instance.\n");

	/* Start Bash */
	if (!spawn_shell(argv_bash, sandbox_environ, print_debug))
		success = 0;

	/* As spawn_shell() free both argv_bash and sandbox_environ, make sure
	 * we do not run into issues in future if we need a OOM error below
	 * this ... */
	argv_bash = NULL;
	sandbox_environ = NULL;

	if (print_debug)
		printf("Cleaning up sandbox process\n");

	if (print_debug) {
		puts(sandbox_banner);
		printf("The protected environment has been shut down.\n");
	}

	if (rc_file_exists(sandbox_info.sandbox_log)) {
		sandbox_log_presence = 1;
		print_sandbox_log(sandbox_info.sandbox_log);
	} else if (print_debug) {
		printf("--------------------------------------------------------------------------------\n");
	}

	if ((sandbox_log_presence) || (!success))
		return 1;
	else
		return 0;

oom_error:
	if (NULL != argv_bash)
		str_list_free(argv_bash);

	perror("sandbox:  Out of memory (environ)");
	exit(EXIT_FAILURE);
}
