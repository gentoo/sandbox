/*
**    Path sandbox for the gentoo linux portage package system, initially
**    based on the ROCK Linux Wrapper for getting a list of created files
**
**  to integrate with bash, bash should have been built like this
**
**  ./configure --prefix=<prefix> --host=<host> --without-gnu-malloc
**
**  it's very important that the --enable-static-link option is NOT specified
**    
**    Copyright (C) 2001 Geert Bevin, Uwyn, http://www.uwyn.com
**    Distributed under the terms of the GNU General Public License, v2 or later 
**    Author : Geert Bevin <gbevin@uwyn.com>
**  $Header$
*/

/* #define _GNU_SOURCE */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "sandbox.h"

struct sandbox_info_t {
	char sandbox_log[SB_PATH_MAX];
	char sandbox_debug_log[SB_PATH_MAX];
	char sandbox_lib[SB_PATH_MAX];
	char sandbox_rc[SB_PATH_MAX];
	char work_dir[SB_PATH_MAX];
	char var_tmp_dir[SB_PATH_MAX];
	char tmp_dir[SB_PATH_MAX];
	char *home_dir;
} sandbox_info_t;

static int print_debug = 0;
static int stop_called = 0;

int sandbox_setup(struct sandbox_info_t *sandbox_info)
{
	if (NULL != getenv(ENV_PORTAGE_TMPDIR)) {
		/* Portage handle setting SANDBOX_WRITE itself. */
		sandbox_info->work_dir[0] = '\0';
	} else {
		if (NULL == getcwd(sandbox_info->work_dir, SB_PATH_MAX)) {
			perror("sandbox:  Failed to get current directory");
			return -1;
		}
	}
	
	/* Do not resolve symlinks, etc .. libsandbox will handle that. */
	if (1 != is_dir(VAR_TMPDIR, 1)) {
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
	if (1 == exists(sandbox_info->sandbox_log)) {
		if (-1 == unlink(sandbox_info->sandbox_log)) {
			perror("sandbox:  Could not unlink old log file");
			return -1;
		}
	}

	/* Generate sandbox debug log full path */
	get_sandbox_debug_log(sandbox_info->sandbox_debug_log);
	if (1 == exists(sandbox_info->sandbox_debug_log)) {
		if (-1 == unlink(sandbox_info->sandbox_debug_log)) {
			perror("sandbox:  Could not unlink old debug log file");
			return -1;
		}
	}

	return 0;
}

int print_sandbox_log(char *sandbox_log)
{
	int sandbox_log_file = -1;
	char *beep_count_env = NULL;
	int i, color, beep_count = 0;
	long len = 0;
	char *buffer = NULL;

	if (1 != is_file(sandbox_log)) {
		perror("sandbox:  Log file is not a regular file");
		return 0;
	}
	
	sandbox_log_file = open(sandbox_log, O_RDONLY);
	if (-1 == sandbox_log_file) {
		perror("sandbox:  Could not open Log file");
		return 0;
	}

	len = file_length(sandbox_log_file);
	buffer = (char *)malloc((len + 1) * sizeof(char));
	memset(buffer, 0, len + 1);
	read(sandbox_log_file, buffer, len);
	close(sandbox_log_file);

	color = ((getenv("NOCOLOR") != NULL) ? 0 : 1);

	EERROR(color,
	       "--------------------------- ACCESS VIOLATION SUMMARY ---------------------------",
	       "\n");
	EERROR(color, "LOG FILE = \"%s\"", "\n\n", sandbox_log);
	fprintf(stderr, "%s", buffer);
	if (NULL != buffer)
		free(buffer);
	EERROR(color,
	       "--------------------------------------------------------------------------------",
	       "\n");

	beep_count_env = getenv(ENV_SANDBOX_BEEP);
	if (beep_count_env)
		beep_count = atoi(beep_count_env);
	else
		beep_count = DEFAULT_BEEP_COUNT;

	for (i = 0; i < beep_count; i++) {
		fputc('\a', stderr);
		if (i < beep_count - 1)
			sleep(1);
	}
	
	return 1;
}

void stop(int signum)
{
	if (stop_called == 0) {
		stop_called = 1;
		printf("sandbox:  Caught signal %d in pid %d\n",
		       signum, getpid());
	} else {
		fprintf(stderr,
			"sandbox:  Signal already caught and busy still cleaning up!\n");
	}
}

int get_sandbox_write_envvar(char *buf, struct sandbox_info_t *sandbox_info)
{
	int retval = 0;
	
	/* bzero out entire buffer then append trailing 0 */
	memset(buf, 0, SB_BUF_LEN);

	/* these could go into make.globals later on */
	retval = snprintf(buf, SB_BUF_LEN,
		 "%s:%s/.gconfd/lock:%s/.bash_history:%s:%s:%s:%s",
		 "/dev/zero:/dev/null:/dev/fd:/proc/self/fd:/dev/pts/:"
		 "/dev/vc/:/dev/pty:/dev/tty:/dev/tts:/dev/console:"
		 "/dev/shm/ngpt:/var/log/scrollkeeper.log:"
		 "/usr/tmp/conftest:/usr/lib/conftest:"
		 "/usr/lib32/conftest:/usr/lib64/conftest:"
		 "/usr/tmp/cf:/usr/lib/cf:/usr/lib32/cf:/usr/lib64/cf",
		 sandbox_info->home_dir, sandbox_info->home_dir,
		 sandbox_info->work_dir[0] != '\0' ? sandbox_info->work_dir
		                                   : "",
		 sandbox_info->tmp_dir, sandbox_info->var_tmp_dir,
		 "/tmp/:/var/tmp/");
	if (SB_BUF_LEN <= retval) {
		errno = EMSGSIZE;
		perror("sandbox:  Failed to generate SANDBOX_WRITE");
		return -1;
	}

	return 0;
}

int get_sandbox_predict_envvar(char *buf, struct sandbox_info_t *sandbox_info)
{
	int retval = 0;
	/* bzero out entire buffer then append trailing 0 */
	memset(buf, 0, SB_BUF_LEN);

	/* these should go into make.globals later on */
	retval = snprintf(buf, SB_BUF_LEN, "%s/.:"
		 "/usr/lib/python2.0/:"
		 "/usr/lib/python2.1/:"
		 "/usr/lib/python2.2/:"
		 "/usr/lib/python2.3/:"
		 "/usr/lib/python2.4/:"
		 "/usr/lib/python2.5/:"
		 "/usr/lib/python3.0/:"
		 "/var/db/aliases.db:"
		 "/var/db/netgroup.db:"
		 "/var/db/netmasks.db:"
		 "/var/db/ethers.db:"
		 "/var/db/rpc.db:"
		 "/var/db/protocols.db:"
		 "/var/db/services.db:"
		 "/var/db/networks.db:"
		 "/var/db/hosts.db:"
		 "/var/db/group.db:"
		 "/var/db/passwd.db",
		 sandbox_info->home_dir);
	if (SB_BUF_LEN <= retval) {
		errno = EMSGSIZE;
		perror("sandbox:  Failed to generate SANDBOX_PREDICT");
		return -1;
	}

	return 0;
}

int sandbox_setenv(char **env, const char *name, const char *val) {
	char **tmp_env = env;
	char *tmp_string = NULL;
	int retval = 0;

	/* XXX: We add the new variable to the end (no replacing).  If this
	 *      is changed, we need to fix sandbox_setup_environ() below */
	while (NULL != *tmp_env)
		tmp_env++;

	/* strlen(name) + strlen(val) + '=' + '\0' */
	/* FIXME: Should probably free this at some stage - more neatness than
	 *        a real leak that will cause issues. */
	tmp_string = calloc(strlen(name) + strlen(val) + 2, sizeof(char *));
	if (NULL == tmp_string) {
		perror("sandbox:  Out of memory (sandbox_setenv)");
		exit(EXIT_FAILURE);
	}

	retval = snprintf(tmp_string, strlen(name) + strlen(val) + 2, "%s=%s",
			  name, val);
	*tmp_env = tmp_string;

	return 0;
}

/* We setup the environment child side only to prevent issues with
 * setting LD_PRELOAD parent side */
char **sandbox_setup_environ(struct sandbox_info_t *sandbox_info)
{
	int env_size = 0;
	int have_ld_preload = 0;
	
	char **new_environ;
	char **env_ptr = environ;
	char sandbox_write_envvar[SB_BUF_LEN];
	char sandbox_predict_envvar[SB_BUF_LEN];
	char *ld_preload_envvar = NULL;
	char *orig_ld_preload_envvar = NULL;

	/* Unset these, as its easier than replacing when setting up our
	 * new environment below */
	unsetenv(ENV_SANDBOX_LIB);
	unsetenv(ENV_SANDBOX_BASHRC);
	unsetenv(ENV_SANDBOX_LOG);
	unsetenv(ENV_SANDBOX_DEBUG_LOG);
	
	if (NULL != getenv(ENV_LD_PRELOAD)) {
		have_ld_preload = 1;
		orig_ld_preload_envvar = getenv(ENV_LD_PRELOAD);

		/* FIXME: Should probably free this at some stage - more neatness
		 *        than a real leak that will cause issues. */
		ld_preload_envvar = calloc(strlen(orig_ld_preload_envvar) +
				strlen(sandbox_info->sandbox_lib) + 2,
				sizeof(char *));
		if (NULL == ld_preload_envvar)
			return NULL;
		snprintf(ld_preload_envvar, strlen(orig_ld_preload_envvar) +
				strlen(sandbox_info->sandbox_lib) + 2, "%s %s",
				sandbox_info->sandbox_lib, orig_ld_preload_envvar);
	} else {
		/* FIXME: Should probably free this at some stage - more neatness
		 *        than a real leak that will cause issues. */
		ld_preload_envvar = strndup(sandbox_info->sandbox_lib,
				strlen(sandbox_info->sandbox_lib));
		if (NULL == ld_preload_envvar)
			return NULL;
	}
	/* Do not unset this, as strange things might happen */
	/* unsetenv(ENV_LD_PRELOAD); */

	while (NULL != *env_ptr) {
		env_size++;
		env_ptr++;
	}

	/* FIXME: Should probably free this at some stage - more neatness than
	 *        a real leak that will cause issues. */
	new_environ = calloc((env_size + 15 + 1) * sizeof(char *), sizeof(char *));
	if (NULL == new_environ)
		return NULL;

	/* First add our new variables to the beginning - this is due to some
	 * weirdness that I cannot remember */
	sandbox_setenv(new_environ, ENV_SANDBOX_LIB, sandbox_info->sandbox_lib);
	sandbox_setenv(new_environ, ENV_SANDBOX_BASHRC, sandbox_info->sandbox_rc);
	sandbox_setenv(new_environ, ENV_SANDBOX_LOG, sandbox_info->sandbox_log);
	sandbox_setenv(new_environ, ENV_SANDBOX_DEBUG_LOG,
			sandbox_info->sandbox_debug_log);
	/* If LD_PRELOAD was not set, set it here, else do it below */
	if (1 != have_ld_preload)
		sandbox_setenv(new_environ, ENV_LD_PRELOAD, ld_preload_envvar);

	if (!getenv(ENV_SANDBOX_DENY))
		sandbox_setenv(new_environ, ENV_SANDBOX_DENY, LD_PRELOAD_FILE);

	if (!getenv(ENV_SANDBOX_READ))
		sandbox_setenv(new_environ, ENV_SANDBOX_READ, "/");

	if (-1 == get_sandbox_write_envvar(sandbox_write_envvar, sandbox_info))
		return NULL;
	if (!getenv(ENV_SANDBOX_WRITE))
		sandbox_setenv(new_environ, ENV_SANDBOX_WRITE, sandbox_write_envvar);

	if (-1 == get_sandbox_predict_envvar(sandbox_predict_envvar, sandbox_info))
		return NULL;
	if (!getenv(ENV_SANDBOX_PREDICT))
		sandbox_setenv(new_environ, ENV_SANDBOX_PREDICT, sandbox_predict_envvar);

	/* Make sure our bashrc gets preference */
	sandbox_setenv(new_environ, ENV_BASH_ENV, sandbox_info->sandbox_rc);

	/* This one should NEVER be set in ebuilds, as it is the one
	 * private thing libsandbox.so use to test if the sandbox
	 * should be active for this pid, or not.
	 *
	 * azarah (3 Aug 2002)
	 */

	sandbox_setenv(new_environ, ENV_SANDBOX_ACTIVE, SANDBOX_ACTIVE);

	env_size = 0;
	while (NULL != new_environ[env_size])
		env_size++;

	/* Now add the rest */
	env_ptr = environ;
	while (NULL != *env_ptr) {
		if ((1 == have_ld_preload) &&
		    (strstr(*env_ptr, LD_PRELOAD_EQ) == *env_ptr))
			/* If LD_PRELOAD was set, and this is it in the original
			 * environment, replace it with our new copy */
			/* XXX: The following works as it just add whatever as
			 *      the last variable to nev_environ */
			sandbox_setenv(new_environ, ENV_LD_PRELOAD,
					ld_preload_envvar);
		else
			new_environ[env_size + (env_ptr - environ)] = *env_ptr;
		env_ptr++;
	}

	return new_environ;
}

int spawn_shell(char *argv_bash[], char *env[], int debug)
{
	int pid;
	int status = 0;
	int ret = 0;

	pid = fork();

	/* Child's process */
	if (0 == pid) {
		execve(argv_bash[0], argv_bash, env);
		return 0;
	} else if (pid < 0) {
		if (debug)
			fprintf(stderr, "Process failed to spawn!\n");
		return 0;
	}
	ret = waitpid(pid, &status, 0);
	if ((-1 == ret) || (status > 0)) {
		if (debug)
			fprintf(stderr, "Process returned with failed exit status!\n");
		return 0;
	}

	return 1;
}

int main(int argc, char **argv)
{
	int i = 0, success = 1;
	int sandbox_log_presence = 0;
	long len;

	struct sandbox_info_t sandbox_info;

	char **sandbox_environ;
	char **argv_bash = NULL;

	char *run_str = "-c";

	/* Only print info if called with no arguments .... */
	if (argc < 2)
		print_debug = 1;

	if (print_debug)
		printf("========================== Gentoo linux path sandbox ===========================\n");

	/* check if a sandbox is already running */
	if ((NULL != getenv(ENV_SANDBOX_ACTIVE)) &&
	    (0 == strncmp(getenv(ENV_SANDBOX_ACTIVE), SANDBOX_ACTIVE, 13))) {
		fprintf(stderr, "Not launching a new sandbox instance\n");
		fprintf(stderr, "Another one is already running in this process hierarchy.\n");
		exit(EXIT_FAILURE);
	}

	/* determine the location of all the sandbox support files */
	if (print_debug)
		printf("Detection of the support files.\n");

	if (-1 == sandbox_setup(&sandbox_info)) {
		fprintf(stderr, "sandbox:  Failed to setup sandbox.");
		exit(EXIT_FAILURE);
	}
	
	/* verify the existance of required files */
	if (print_debug)
		printf("Verification of the required files.\n");

#ifndef SB_HAVE_MULTILIB
	if (0 >= exists(sandbox_info.sandbox_lib)) {
		perror("sandbox:  Could not open the sandbox library");
		exit(EXIT_FAILURE);
	}
#endif
	if (0 >= exists(sandbox_info.sandbox_rc)) {
		perror("sandbox:  Could not open the sandbox rc file");
		exit(EXIT_FAILURE);
	}

	/* set up the required environment variables */
	if (print_debug)
		printf("Setting up the required environment variables.\n");

	/* This one should not be child only, as we check above to see
	 * if we are already running (check sandbox_setup_environ).
	 * This needs to be set before calling sandbox_setup_environ(),
	 * else its not set for the child */
	setenv(ENV_SANDBOX_ON, "1", 0);

	/* Setup the child environment stuff */
	sandbox_environ = sandbox_setup_environ(&sandbox_info);
	if (NULL == sandbox_environ) {
		perror("sandbox:  Out of memory (environ)");
		exit(EXIT_FAILURE);
	}

	/* If not in portage, cd into it work directory */
	if ('\0' != sandbox_info.work_dir[0])
		chdir(sandbox_info.work_dir);

	argv_bash = (char **)malloc(6 * sizeof(char *));
	argv_bash[0] = strdup("/bin/bash");
	argv_bash[1] = strdup("-rcfile");
	argv_bash[2] = strdup(sandbox_info.sandbox_rc);

	if (argc < 2)
		argv_bash[3] = NULL;
	else
		argv_bash[3] = strdup(run_str);	/* "-c" */

	argv_bash[4] = NULL;	/* strdup(run_arg); */
	argv_bash[5] = NULL;

	if (argc >= 2) {
		for (i = 1; i < argc; i++) {
			if (NULL == argv_bash[4])
				len = 0;
			else
				len = strlen(argv_bash[4]);

			argv_bash[4] = (char *)realloc(argv_bash[4],
					(len + strlen(argv[i]) + 2) * sizeof(char));

			if (0 == len)
				argv_bash[4][0] = 0;
			if (1 != i)
				strcat(argv_bash[4], " ");

			strcat(argv_bash[4], argv[i]);
		}
	}

	/* set up the required signal handlers */
	signal(SIGHUP, &stop);
	signal(SIGINT, &stop);
	signal(SIGQUIT, &stop);
	signal(SIGTERM, &stop);

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

	/* Free bash stuff */
	for (i = 0; i < 6; i++) {
		if (argv_bash[i])
			free(argv_bash[i]);
		argv_bash[i] = NULL;
	}
	if (argv_bash)
		free(argv_bash);
	argv_bash = NULL;

	if (print_debug)
		printf("Cleaning up sandbox process\n");

	if (print_debug) {
		printf("========================== Gentoo linux path sandbox ===========================\n");
		printf("The protected environment has been shut down.\n");
	}

	if (1 == exists(sandbox_info.sandbox_log)) {
		sandbox_log_presence = 1;
		print_sandbox_log(sandbox_info.sandbox_log);
	} else if (print_debug) {
		printf("--------------------------------------------------------------------------------\n");
	}

	if ((sandbox_log_presence) || (!success))
		return 1;
	else
		return 0;
}

// vim:noexpandtab noai:cindent ai
