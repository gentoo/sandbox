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
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "sbutil.h"

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

volatile static int stop_called = 0;
volatile static pid_t child_pid = 0;

static char log_domain[] = "sandbox";

extern char **environ;

int sandbox_setup(struct sandbox_info_t *sandbox_info, bool interactive)
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
	if (!rc_is_dir(VAR_TMPDIR, TRUE)) {
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

int print_sandbox_log(char *sandbox_log)
{
	int sandbox_log_file = -1;
	char *beep_count_env = NULL;
	int i, color, beep_count = 0;
	off_t len = 0;
	char *buffer = NULL;

	if (!rc_is_file(sandbox_log, FALSE)) {
		perror("sandbox:  Log file is not a regular file");
		return 0;
	}
	
	sandbox_log_file = sb_open(sandbox_log, O_RDONLY, 0);
	if (-1 == sandbox_log_file) {
		perror("sandbox:  Could not open Log file");
		return 0;
	}

	len = rc_get_size(sandbox_log, TRUE);
	if (0 == len)
		return 0;
	buffer = (char *)xmalloc((len + 1) * sizeof(char));
	if (NULL == buffer) {
		perror("sandbox:  Could not allocate buffer for Log file");
		return 0;
	}
	memset(buffer, 0, len + 1);
	if (-1 == sb_read(sandbox_log_file, buffer, len)) {
		perror("sandbox:  Could read Log file");
		return 0;
	}
	sb_close(sandbox_log_file);

	color = ((is_env_on(ENV_NOCOLOR)) ? 0 : 1);

	SB_EERROR(color,
	       "--------------------------- ACCESS VIOLATION SUMMARY ---------------------------",
	       "\n");
	SB_EERROR(color, "LOG FILE = \"%s\"", "\n\n", sandbox_log);
	fprintf(stderr, "%s", buffer);
	free(buffer);
	SB_EERROR(color,
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
	if (0 == stop_called) {
		stop_called = 1;
		printf("sandbox:  Caught signal %d in pid %d\n",
		       signum, getpid());
	} else {
		fprintf(stderr,
			"sandbox:  Signal already caught and busy still cleaning up!\n");
	}
}

void usr1_handler(int signum, siginfo_t *siginfo, void *ucontext)
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

char *sandbox_subst_env_vars(dyn_buf_t *env_data)
{
	dyn_buf_t *new_data = NULL;
	char *tmp_ptr, *tmp_data = NULL;
	char *var_start, *var_stop;

	new_data = new_dyn_buf();
	if (NULL == new_data)
		return NULL;

	tmp_data = read_line_dyn_buf(env_data);
	if (NULL == tmp_data)
		goto error;
	tmp_ptr = tmp_data;

	while (NULL != (var_start = strchr(tmp_ptr, '$'))) {
		char *env = NULL;

		var_stop = strchr(var_start, '}');

		/* We only support ${} style env var names, so just skip any
		 * '$' that do not follow this syntax */
		if (('{' != var_start[1]) || (NULL == var_stop)) {
		  	tmp_ptr = var_start + 1;
			continue;
		}
		
		/* Terminate part before env string so that we can copy it */
		var_start[0] = '\0';
		/* Move var_start past '${' */
		var_start += 2;
		/* Terminate the name of the env var */
		var_stop[0] = '\0';

		if (strlen(var_start) > 0)
			env = getenv(var_start);
		if (-1 == sprintf_dyn_buf(new_data, "%s%s",
					  tmp_ptr ? tmp_ptr : "",
					  env ? env : ""))
			goto error;

		/* Move tmp_ptr past the '}' of the env var */
		tmp_ptr = var_stop + 1;
	}

	if (0 != strlen(tmp_ptr))
		if (-1 == write_dyn_buf(new_data, tmp_ptr, strlen(tmp_ptr)))
			goto error;

	free(tmp_data);

	tmp_data = read_line_dyn_buf(new_data);
	if (NULL == tmp_data)
		goto error;

	free_dyn_buf(new_data);

	return tmp_data;

error:
	if (NULL != new_data)
		free_dyn_buf(new_data);
	if (NULL != tmp_data)
		free(tmp_data);

	return NULL;
}

void sandbox_set_env_var(const char *env_var)
{
	char *config;

	/* We check if the variable is set in the environment, and if not, we
	 * get it from sandbox.conf, and if they exist, we just add them to the
	 * environment if not already present. */
	config = rc_get_cnf_entry(SANDBOX_CONF_FILE, env_var, NULL);
	if (NULL != config) {
		setenv(ENV_SANDBOX_VERBOSE, config, 0);
		free(config);
	}
}

int sandbox_set_env_access_var(const char *access_var)
{
	dyn_buf_t *env_data = NULL;
  	int count = 0;
	char *config = NULL;
	char **confd_files = NULL;
	bool use_confd = TRUE;

	env_data = new_dyn_buf();
	if (NULL == env_data)
		return -1;

	/* Now get the defaults for the access variable from sandbox.conf.
	 * These do not get overridden via the environment. */
	config = rc_get_cnf_entry(SANDBOX_CONF_FILE, access_var, ":");
	if (NULL != config) {
		if (-1 == write_dyn_buf(env_data, config, strlen(config)))
			goto error;
		free(config);
		config = NULL;
	}
	/* Append whatever might be already set.  If anything is set, we do
	 * not process the sandbox.d/ files for this variable. */
	if (NULL != getenv(access_var)) {
		use_confd = FALSE;
		if (-1 == sprintf_dyn_buf(env_data, env_data->wr_index ? ":%s" : "%s",
					  getenv(access_var)))
			goto error;
	}

	if (!use_confd)
		goto done;

	/* Now scan the files in sandbox.d/ if the access variable was not
	 * alreay set. */
	confd_files = rc_ls_dir(SANDBOX_CONFD_DIR, FALSE, TRUE);
	if (NULL != confd_files) {
		while (NULL != confd_files[count]) {
			config = rc_get_cnf_entry(confd_files[count], access_var, ":");
			if (NULL != config) {
				if (-1 == sprintf_dyn_buf(env_data,
							  env_data->wr_index ? ":%s" : "%s",
							  config))
					goto error;
				free(config);
				config = NULL;
			}
			count++;
		}

		str_list_free(confd_files);
	}

done:
	if (env_data->wr_index > 0) {
	  	char *subst;

		subst = sandbox_subst_env_vars(env_data);
		if (NULL == subst)
			goto error;

		setenv(access_var, subst, 1);
		free(subst);
	}

	free_dyn_buf(env_data);

	return 0;

error:
	if (NULL != env_data)
		free_dyn_buf(env_data);
	if (NULL != config)
		free(config);
	if (NULL != confd_files)
		str_list_free(confd_files);

	return -1;
}

int sandbox_setup_env_config(struct sandbox_info_t *sandbox_info)
{
	sandbox_set_env_var(ENV_SANDBOX_VERBOSE);
	sandbox_set_env_var(ENV_SANDBOX_DEBUG);
	sandbox_set_env_var(ENV_SANDBOX_BEEP);
	sandbox_set_env_var(ENV_NOCOLOR);

	if (-1 == sandbox_set_env_access_var(ENV_SANDBOX_DENY))
		return -1;
	if (NULL == getenv(ENV_SANDBOX_DENY))
		setenv(ENV_SANDBOX_DENY, LD_PRELOAD_FILE, 1);

	if (-1 == sandbox_set_env_access_var(ENV_SANDBOX_READ))
		return -1;
	if (NULL == getenv(ENV_SANDBOX_READ))
		setenv(ENV_SANDBOX_READ, "/", 1);

	if (-1 == sandbox_set_env_access_var(ENV_SANDBOX_WRITE))
		return -1;
	if ((NULL == getenv(ENV_SANDBOX_WRITE)) &&
	    (NULL != sandbox_info->work_dir))
		setenv(ENV_SANDBOX_WRITE, sandbox_info->work_dir, 1);

	if (-1 == sandbox_set_env_access_var(ENV_SANDBOX_PREDICT))
		return -1;
	if ((NULL == getenv(ENV_SANDBOX_PREDICT)) &&
	    (NULL != sandbox_info->home_dir))
		setenv(ENV_SANDBOX_PREDICT, sandbox_info->home_dir, 1);

	return 0;
}

int sandbox_setenv(char ***envp, const char *name, const char *val) {
	char *tmp_string = NULL;

	/* strlen(name) + strlen(val) + '=' + '\0' */
	tmp_string = xmalloc((strlen(name) + strlen(val) + 2) * sizeof(char));
	if (NULL == tmp_string)
		goto error;

	snprintf(tmp_string, strlen(name) + strlen(val) + 2,
		 "%s=%s", name, val);

	str_list_add_item((*envp), tmp_string, error);

	return 0;

error:
	perror("sandbox:  Out of memory (sandbox_setenv)");
	exit(EXIT_FAILURE);
}

/* We setup the environment child side only to prevent issues with
 * setting LD_PRELOAD parent side */
char **sandbox_setup_environ(struct sandbox_info_t *sandbox_info, bool interactive)
{
	int have_ld_preload = 0;
	
	char **new_environ = NULL;
	char **env_ptr;
	char *ld_preload_envvar = NULL;
	char *orig_ld_preload_envvar = NULL;
	char sb_pid[64];

	if (-1 == sandbox_setup_env_config(sandbox_info))
		return NULL;

	/* Unset these, as its easier than replacing when setting up our
	 * new environment below */
	unsetenv(ENV_SANDBOX_ON);
	unsetenv(ENV_SANDBOX_PID);
	unsetenv(ENV_SANDBOX_LIB);
	unsetenv(ENV_SANDBOX_BASHRC);
	unsetenv(ENV_SANDBOX_LOG);
	unsetenv(ENV_SANDBOX_DEBUG_LOG);
	unsetenv(ENV_SANDBOX_WORKDIR);
	unsetenv(ENV_SANDBOX_ACTIVE);
	unsetenv(ENV_SANDBOX_INTRACTV);
	unsetenv(ENV_BASH_ENV);
	
	if (NULL != getenv(ENV_LD_PRELOAD)) {
		have_ld_preload = 1;
		orig_ld_preload_envvar = getenv(ENV_LD_PRELOAD);

		ld_preload_envvar = xcalloc(strlen(orig_ld_preload_envvar) +
				strlen(sandbox_info->sandbox_lib) + 2,
				sizeof(char));
		if (NULL == ld_preload_envvar)
			return NULL;
		snprintf(ld_preload_envvar, strlen(orig_ld_preload_envvar) +
				strlen(sandbox_info->sandbox_lib) + 2, "%s %s",
				sandbox_info->sandbox_lib, orig_ld_preload_envvar);
	} else {
		ld_preload_envvar = rc_strndup(sandbox_info->sandbox_lib,
				strlen(sandbox_info->sandbox_lib));
		if (NULL == ld_preload_envvar)
			return NULL;
	}
	/* Do not unset this, as strange things might happen */
	/* unsetenv(ENV_LD_PRELOAD); */

	snprintf(sb_pid, sizeof(sb_pid), "%i", getpid());
	
	/* First add our new variables to the beginning - this is due to some
	 * weirdness that I cannot remember */
	sandbox_setenv(&new_environ, ENV_SANDBOX_ON, "1");
	sandbox_setenv(&new_environ, ENV_SANDBOX_PID, sb_pid);
	sandbox_setenv(&new_environ, ENV_SANDBOX_LIB, sandbox_info->sandbox_lib);
	sandbox_setenv(&new_environ, ENV_SANDBOX_BASHRC, sandbox_info->sandbox_rc);
	sandbox_setenv(&new_environ, ENV_SANDBOX_LOG, sandbox_info->sandbox_log);
	sandbox_setenv(&new_environ, ENV_SANDBOX_DEBUG_LOG,
			sandbox_info->sandbox_debug_log);
	/* Is this an interactive session? */
	if (interactive)
		sandbox_setenv(&new_environ, ENV_SANDBOX_INTRACTV, "1");
	/* Just set the these if not already set so that is_env_on() work */
	if (!getenv(ENV_SANDBOX_VERBOSE))
		sandbox_setenv(&new_environ, ENV_SANDBOX_VERBOSE, "1");
	if (!getenv(ENV_SANDBOX_DEBUG))
		sandbox_setenv(&new_environ, ENV_SANDBOX_DEBUG, "0");
	if (!getenv(ENV_NOCOLOR))
		sandbox_setenv(&new_environ, ENV_NOCOLOR, "no");
	/* If LD_PRELOAD was not set, set it here, else do it below */
	if (1 != have_ld_preload)
		sandbox_setenv(&new_environ, ENV_LD_PRELOAD, ld_preload_envvar);

	/* Make sure our bashrc gets preference */
	sandbox_setenv(&new_environ, ENV_BASH_ENV, sandbox_info->sandbox_rc);

	/* This one should NEVER be set in ebuilds, as it is the one
	 * private thing libsandbox.so use to test if the sandbox
	 * should be active for this pid, or not.
	 *
	 * azarah (3 Aug 2002)
	 */

	sandbox_setenv(&new_environ, ENV_SANDBOX_ACTIVE, SANDBOX_ACTIVE);

	/* Now add the rest */
	env_ptr = environ;
	while (NULL != *env_ptr) {
		if ((1 == have_ld_preload) &&
		    (strstr(*env_ptr, LD_PRELOAD_EQ) == *env_ptr))
			/* If LD_PRELOAD was set, and this is it in the original
			 * environment, replace it with our new copy */
			/* XXX: The following works as it just add whatever as
			 *      the last variable to nev_environ */
			sandbox_setenv(&new_environ, ENV_LD_PRELOAD,
					ld_preload_envvar);
		else
		  	str_list_add_item_copy(new_environ, (*env_ptr), error);

		env_ptr++;
	}

	if (NULL != ld_preload_envvar)
		free(ld_preload_envvar);

	return new_environ;

error:
	if (NULL != new_environ)
		str_list_free(new_environ);
	if (NULL != ld_preload_envvar)
		free(ld_preload_envvar);

	return NULL;
}

int spawn_shell(char *argv_bash[], char **env, int debug)
{
	int status = 0;
	int ret = 0;

	child_pid = fork();

	/* Child's process */
	if (0 == child_pid) {
		execve(argv_bash[0], argv_bash, env);
		_exit(EXIT_FAILURE);
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
	if ((-1 == ret) || (status > 0)) {
		if (debug)
			fprintf(stderr, "Process returned with failed exit status!\n");
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

	if (print_debug)
		printf("========================== Gentoo linux path sandbox ===========================\n");

	/* check if a sandbox is already running */
	if (NULL != getenv(ENV_SANDBOX_ACTIVE)) {
		fprintf(stderr, "Not launching a new sandbox instance\n");
		fprintf(stderr, "Another one is already running in this process hierarchy.\n");
		exit(EXIT_FAILURE);
	}

	/* determine the location of all the sandbox support files */
	if (print_debug)
		printf("Detection of the support files.\n");

	if (-1 == sandbox_setup(&sandbox_info, print_debug)) {
		fprintf(stderr, "sandbox:  Failed to setup sandbox.");
		exit(EXIT_FAILURE);
	}
	
	/* verify the existance of required files */
	if (print_debug)
		printf("Verification of the required files.\n");

#ifndef SB_HAVE_MULTILIB
	if (!rc_file_exists(sandbox_info.sandbox_lib)) {
		perror("sandbox:  Could not open the sandbox library");
		exit(EXIT_FAILURE);
	}
#endif
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

			snprintf(argv_bash[4] + strlen(argv_bash[4]),
				 strlen(argv[i]) + 2, " %s",
				 argv[i]);
		}
	}

	/* Setup the child environment stuff.
	 * XXX:  We free this in spawn_shell(). */
	sandbox_environ = sandbox_setup_environ(&sandbox_info, print_debug);
	if (NULL == sandbox_environ)
		goto oom_error;

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
		printf("========================== Gentoo linux path sandbox ===========================\n");
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

// vim:noexpandtab noai:cindent ai
