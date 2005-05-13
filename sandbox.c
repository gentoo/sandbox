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
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include "sandbox.h"

int cleaned_up = 0;
int print_debug = 0;
int stop_called = 0;

/* Read pids file, and load active pids into an array.	Return number of pids in array */
int load_active_pids(int fd, int **pids)
{
	char *data = NULL;
	char *ptr = NULL, *ptr2 = NULL;
	int my_pid;
	int num_pids = 0;
	long len;

	pids[0] = NULL;

	len = file_length(fd);

	/* Allocate and zero datablock to read pids file */
	data = (char *)malloc((len + 1) * sizeof(char));
	memset(data, 0, len + 1);

	/* Start at beginning of file */
	lseek(fd, 0L, SEEK_SET);

	/* read entire file into a buffer */
	read(fd, data, len);

	ptr = data;

	/* Loop and read all pids */
	while (1) {
		/* Find new line */
		ptr2 = strchr(ptr, '\n');
		if (ptr2 == NULL)
			break;	/* No more PIDs */

		/* Clear the \n. And  ptr  should have a null-terminated decimal string */
		ptr2[0] = 0;

		my_pid = atoi(ptr);

		/* If the PID is still alive, add it to our array */
		if ((0 != my_pid) && (0 == kill(my_pid, 0))) {
			pids[0] = (int *)realloc(pids[0], (num_pids + 1) * sizeof(int));
			pids[0][num_pids] = my_pid;
			num_pids++;
		}

		/* Put ptr past the NULL we just wrote */
		ptr = ptr2 + 1;
	}

	if (data)
		free(data);
	data = NULL;

	return num_pids;
}

void cleanup()
{
	int i = 0;
	int success = 1;
	int pids_file = -1, num_of_pids = 0;
	int *pids_array = NULL;
	char pid_string[SB_BUF_LEN];
	char *sandbox_pids_file;

	/* Generate sandbox pids-file path */
	sandbox_pids_file = get_sandbox_pids_file();

	/* Remove this sandbox's bash pid from the global pids
	 * file if we have not already done so */
	if (0 == cleaned_up) {
		cleaned_up = 1;
		success = 1;

		if (print_debug)
			printf("Cleaning up pids file.\n");

		/* Stat the PIDs file, make sure it exists and is a regular file */
		if (file_exist(sandbox_pids_file, 1) <= 0) {
			fprintf(stderr, ">>> pids file is not a regular file\n");
			success = 0;
			/* We should really not fail if the pidsfile is missing here, but
			 * rather just exit cleanly, as there is still some cleanup to do */
			return;
		}

		pids_file = file_open(sandbox_pids_file, "r+", 1, 0664, "portage");
		if (-1 == pids_file) {
			success = 0;
			/* Nothing more to do here */
			return;
		}

		/* Load "still active" pids into an array */
		num_of_pids = load_active_pids(pids_file, &pids_array);
		//printf("pids: %d\r\n", num_of_pids);


		file_truncate(pids_file);

		/* if pids are still running, write only the running pids back to the file */
		if (num_of_pids > 1) {
			for (i = 0; i < num_of_pids; i++) {
				if (pids_array[i] != getpid()) {
					sprintf(pid_string, "%d\n", pids_array[i]);

					if (write(pids_file, pid_string, strlen(pid_string)) != strlen(pid_string)) {
						perror(">>> pids file write");
						success = 0;
						break;
					}
				}
			}

			file_close(pids_file);
			pids_file = -1;
		} else {

			file_close(pids_file);
			pids_file = -1;

			/* remove the pidsfile, as this was the last sandbox */
			unlink(sandbox_pids_file);
		}

		if (pids_array != NULL)
			free(pids_array);
		pids_array = NULL;
	}

	free(sandbox_pids_file);
	if (0 == success)
		return;
}

int print_sandbox_log(char *sandbox_log)
{
	int sandbox_log_file = -1;
	char *beep_count_env = NULL;
	int i, color, beep_count = 0;
	long len = 0;
	char *buffer = NULL;

	sandbox_log_file = file_open(sandbox_log, "r", 1, 0664, "portage");
	if (-1 == sandbox_log_file)
		return 0;

	len = file_length(sandbox_log_file);
	buffer = (char *)malloc((len + 1) * sizeof(char));
	memset(buffer, 0, len + 1);
	read(sandbox_log_file, buffer, len);
	file_close(sandbox_log_file);

	color = ((getenv("NOCOLOR") != NULL) ? 0 : 1);

	if (color)
		printf("\e[31;01m");
	printf("--------------------------- ACCESS VIOLATION SUMMARY ---------------------------");
	if (color)
		printf("\033[0m");
	if (color)
		printf("\e[31;01m");
	printf("\nLOG FILE = \"%s\"", sandbox_log);
	if (color)
		printf("\033[0m");
	printf("\n\n");
	printf("%s", buffer);
	if (buffer)
		free(buffer);
	buffer = NULL;
	printf("\e[31;01m--------------------------------------------------------------------------------\033[0m\n");

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
		printf("Caught signal %d in pid %d\r\n", signum, getpid());
		cleanup();
	} else {
		fprintf(stderr, "Pid %d alreadly caught signal and is still cleaning up\n", getpid());
	}
}

void get_sandbox_write_envvar(char *buf, char *home_dir, char *portage_tmp_dir, char *var_tmp_dir, char *tmp_dir)
{
	/* bzero out entire buffer then append trailing 0 */
	memset(buf, 0, SB_BUF_LEN);

	/* these could go into make.globals later on */
	snprintf(buf, SB_BUF_LEN,
		 "%s:%s/.gconfd/lock:%s/.bash_history:%s:%s:%s:%s",
		 "/dev/zero:/dev/fd/:/dev/null:/dev/pts/:"
		 "/dev/vc/:/dev/pty:/dev/tty:"
		 "/dev/shm/ngpt:/var/log/scrollkeeper.log:"
		 "/usr/tmp/conftest:/usr/lib/conftest:"
		 "/usr/lib32/conftest:/usr/lib64/conftest:"
		 "/usr/tmp/cf:/usr/lib/cf:/usr/lib32/cf:/usr/lib64/cf",
		 home_dir, home_dir,
		 (NULL != portage_tmp_dir) ? portage_tmp_dir : tmp_dir,
		 tmp_dir, var_tmp_dir, "/tmp/:/var/tmp/");
}

void get_sandbox_predict_envvar(char *buf, char *home_dir)
{
	/* bzero out entire buffer then append trailing 0 */
	memset(buf, 0, SB_BUF_LEN);

	/* these should go into make.globals later on */
	snprintf(buf, SB_BUF_LEN, "%s/.:"
		 "/usr/lib/python2.0/:"
		 "/usr/lib/python2.1/:"
		 "/usr/lib/python2.2/:"
		 "/usr/lib/python2.3/:"
		 "/usr/lib/python2.4/:"
		 "/usr/lib/python2.5/:"
		 "/usr/lib/python3.0/:",
		 home_dir);
}

int sandbox_setenv(char **env, char *name, char *val) {
	char **tmp_env = env;
	char *tmp_string = NULL;

	while (NULL != *tmp_env)
		tmp_env++;

	/* strlen(name) + strlen(val) + '=' + '\0' */
	/* FIXME: Should probably free this at some stage - more neatness than
	 *        a real leak that will cause issues. */
	tmp_string = calloc(strlen(name) + strlen(val) + 2, sizeof(char *));
	if (NULL == tmp_string) {
		perror(">>> out of memory (sandbox_setenv)");
		exit(1);
	}

	snprintf(tmp_string, strlen(name) + strlen(val) + 2, "%s=%s",
			name, val);
	*tmp_env = tmp_string;

	return 0;
}

/* We setup the environment child side only to prevent issues with
 * setting LD_PRELOAD parent side */
char **sandbox_setup_environ(char *sandbox_dir, char *sandbox_lib, char *sandbox_rc, char *sandbox_log,
		char *sandbox_debug_log, char *sandbox_write_envvar, char *sandbox_predict_envvar)
{
	int env_size = 0;
	
	char **new_environ;
	char **env_ptr = environ;
	char *ld_preload_envvar = NULL;

	/* Unset these, as its easier than replacing when setting up our
	 * new environment below */
	unsetenv(ENV_SANDBOX_DIR);
	unsetenv(ENV_SANDBOX_LIB);
	unsetenv(ENV_SANDBOX_BASHRC);
	unsetenv(ENV_SANDBOX_LOG);
	unsetenv(ENV_SANDBOX_DEBUG_LOG);
	
	if (NULL != getenv("LD_PRELOAD")) {
		/* FIXME: Should probably free this at some stage - more neatness
		 *        than a real leak that will cause issues. */
		ld_preload_envvar = malloc(strlen(getenv("LD_PRELOAD")) +
				strlen(sandbox_lib) + 2);
		if (NULL == ld_preload_envvar)
			return NULL;
		strncpy(ld_preload_envvar, sandbox_lib, strlen(sandbox_lib));
		strncat(ld_preload_envvar, " ", 1);
		strncat(ld_preload_envvar, getenv("LD_PRELOAD"),
				strlen(getenv("LD_PRELOAD")));
	} else {
		/* FIXME: Should probably free this at some stage - more neatness
		 *        than a real leak that will cause issues. */
		ld_preload_envvar = strndup(sandbox_lib, strlen(sandbox_lib));
		if (NULL == ld_preload_envvar)
			return NULL;
	}
	unsetenv("LD_PRELOAD");

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
	sandbox_setenv(new_environ, ENV_SANDBOX_DIR, sandbox_dir);
	sandbox_setenv(new_environ, ENV_SANDBOX_LIB, sandbox_lib);
	sandbox_setenv(new_environ, ENV_SANDBOX_BASHRC, sandbox_rc);
	sandbox_setenv(new_environ, ENV_SANDBOX_LOG, sandbox_log);
	sandbox_setenv(new_environ, ENV_SANDBOX_DEBUG_LOG, sandbox_debug_log);
	sandbox_setenv(new_environ, "LD_PRELOAD", ld_preload_envvar);

	if (!getenv(ENV_SANDBOX_DENY))
		sandbox_setenv(new_environ, ENV_SANDBOX_DENY, LD_PRELOAD_FILE);

	if (!getenv(ENV_SANDBOX_READ))
		sandbox_setenv(new_environ, ENV_SANDBOX_READ, "/");

	if (!getenv(ENV_SANDBOX_WRITE))
		sandbox_setenv(new_environ, ENV_SANDBOX_WRITE, sandbox_write_envvar);

	if (!getenv(ENV_SANDBOX_PREDICT))
		sandbox_setenv(new_environ, ENV_SANDBOX_PREDICT, sandbox_predict_envvar);

	/* This one should NEVER be set in ebuilds, as it is the one
	 * private thing libsandbox.so use to test if the sandbox
	 * should be active for this pid, or not.
	 *
	 * azarah (3 Aug 2002)
	 */

	sandbox_setenv(new_environ, "SANDBOX_ACTIVE", "armedandready");

	env_size = 0;
	while (NULL != new_environ[env_size])
		env_size++;

	/* Now add the rest */
	env_ptr = environ;
	while (NULL != *env_ptr) {
		new_environ[env_size + (env_ptr - environ)] = *env_ptr;
		env_ptr++;
	}

	return new_environ;
}

int spawn_shell(char *argv_bash[], char *env[])
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
		return 0;
	}
	ret = waitpid(pid, &status, 0);
	if ((-1 == ret) || (status > 0))
		return 0;

	return 1;
}

int main(int argc, char **argv)
{
	int ret = 0, i = 0, success = 1;
	int sandbox_log_presence = 0;
	int pids_file = -1;
	long len;

	int *pids_array = NULL;
	int num_of_pids = 0;

	// char run_arg[255];
	char **sandbox_environ;
	char sandbox_log[SB_PATH_MAX];
	char sandbox_debug_log[SB_PATH_MAX];
	char sandbox_dir[SB_PATH_MAX];
	char sandbox_lib[SB_PATH_MAX];
	char sandbox_rc[SB_PATH_MAX];
	char *sandbox_pids_file;
	char portage_tmp_dir[SB_PATH_MAX];
	char var_tmp_dir[SB_PATH_MAX];
	char tmp_dir[SB_PATH_MAX];
	char sandbox_write_envvar[SB_BUF_LEN];
	char sandbox_predict_envvar[SB_BUF_LEN];
	char pid_string[SB_BUF_LEN];
	char **argv_bash = NULL;

	char *run_str = "-c";
	char *home_dir = NULL;
	char *tmp_string = NULL;

	/* Only print info if called with no arguments .... */
	if (argc < 2)
		print_debug = 1;

	if (print_debug)
		printf("========================== Gentoo linux path sandbox ===========================\n");

	/* check if a sandbox is already running */
	if (NULL != getenv(ENV_SANDBOX_ON)) {
		fprintf(stderr, "Not launching a new sandbox instance\n");
		fprintf(stderr, "Another one is already running in this process hierarchy.\n");
		exit(1);
	} else {

		/* determine the location of all the sandbox support files */
		if (print_debug)
			printf("Detection of the support files.\n");

		/* Generate base sandbox path */
		snprintf(sandbox_dir, SB_PATH_MAX, "%s/",
				get_sandbox_path(argv[0]));

		/* Generate sandbox lib path */
		snprintf(sandbox_lib, SB_PATH_MAX, "%s",
				get_sandbox_lib(sandbox_dir));

		/* Generate sandbox pids-file path */
		sandbox_pids_file = get_sandbox_pids_file();

		/* Generate sandbox bashrc path */
		snprintf(sandbox_rc, SB_PATH_MAX, "%s",
				get_sandbox_rc(sandbox_dir));

		/* verify the existance of required files */
		if (print_debug)
			printf("Verification of the required files.\n");

#ifndef SB_HAVE_64BIT_ARCH
		if (file_exist(sandbox_lib, 0) <= 0) {
			fprintf(stderr, "Could not open the sandbox library at '%s'.\n", sandbox_lib);
			return -1;
		}
#endif
		if (file_exist(sandbox_rc, 0) <= 0) {
			fprintf(stderr, "Could not open the sandbox rc file at '%s'.\n", sandbox_rc);
			return -1;
		}

		/* set up the required environment variables */
		if (print_debug)
			printf("Setting up the required environment variables.\n");

		/* Generate sandbox log full path */
		snprintf(sandbox_log, SB_PATH_MAX, "%s",
				get_sandbox_log());

		/* Generate sandbox debug log full path */
		snprintf(sandbox_debug_log, SB_PATH_MAX, "%s",
				get_sandbox_debug_log());

		home_dir = getenv("HOME");
		if (!home_dir) {
			home_dir = "/tmp";
			setenv("HOME", home_dir, 1);
		}

		if (NULL == realpath(getenv("PORTAGE_TMPDIR") ? getenv("PORTAGE_TMPDIR")
		                                              : "/var/tmp/portage",
					portage_tmp_dir)) {
			perror(">>> get portage_tmp_dir");
			exit(1);
		}
		if (NULL == realpath("/var/tmp", var_tmp_dir)) {
			perror(">>> get var_tmp_dir");
			exit(1);
		}
		if (NULL == realpath("/tmp", tmp_dir)) {
			perror(">>> get tmp_dir");
			exit(1);
		}

		/* This one should not be child only, as we check above to see
		 * if we are already running (check sandbox_setup_environ).
		 * This needs to be set before calling sandbox_setup_environ(),
		 * else its not set for the child */
		setenv(ENV_SANDBOX_ON, "1", 0);

		/* Setup the child environment stuff */
		get_sandbox_write_envvar(sandbox_write_envvar, home_dir,
				portage_tmp_dir, var_tmp_dir, tmp_dir);
		get_sandbox_predict_envvar(sandbox_predict_envvar, home_dir);
		sandbox_environ = sandbox_setup_environ(sandbox_dir, sandbox_lib,
				sandbox_rc, sandbox_log, sandbox_debug_log,
				sandbox_write_envvar, sandbox_predict_envvar);
		if (NULL == sandbox_environ) {
			perror(">>> out of memory (environ)");
			exit(1);
		}

		/* if the portage temp dir was present, cd into it */
		if (NULL != portage_tmp_dir)
			chdir(portage_tmp_dir);

		argv_bash = (char **)malloc(6 * sizeof(char *));
		argv_bash[0] = strdup("/bin/bash");
		argv_bash[1] = strdup("-rcfile");
		argv_bash[2] = strdup(sandbox_rc);

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

		/* Load our PID into PIDs file */
		success = 1;
		if (file_exist(sandbox_pids_file, 1) < 0) {
			success = 0;
			fprintf(stderr, ">>> %s is not a regular file\n", sandbox_pids_file);
		} else {
			pids_file = file_open(sandbox_pids_file, "r+", 1, 0664, "portage");
			if (-1 == pids_file)
				success = 0;
		}
		if (1 == success) {
			/* Grab still active pids */
			num_of_pids = load_active_pids(pids_file, &pids_array);

			/* Zero out file */
			file_truncate(pids_file);

			/* Output active pids, and append our pid */
			for (i = 0; i < num_of_pids + 1; i++) {
				/* Time for our entry */
				if (i == num_of_pids)
					sprintf(pid_string, "%d\n", getpid());
				else
					sprintf(pid_string, "%d\n", pids_array[i]);

				if (write(pids_file, pid_string, strlen(pid_string)) != strlen(pid_string)) {
					perror(">>> pids file write");
					success = 0;
					break;
				}
			}
			/* Clean pids_array */
			if (pids_array)
				free(pids_array);
			pids_array = NULL;
			num_of_pids = 0;

			/* We're done with the pids file */
			file_close(pids_file);
		}

		/* Something went wrong, bail out */
		if (0 == success) {
			perror(">>> pids file write");
			exit(1);
		}

		/* STARTING PROTECTED ENVIRONMENT */
		if (print_debug) {
			printf("The protected environment has been started.\n");
			printf("--------------------------------------------------------------------------------\n");
		}

		if (print_debug)
			printf("Shell being started in forked process.\n");

		/* Start Bash */
		if (!spawn_shell(argv_bash, sandbox_environ)) {
			if (print_debug)
				fprintf(stderr, ">>> shell process failed to spawn\n");
			success = 0;
		}

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

		cleanup();

		if (print_debug) {
			printf("========================== Gentoo linux path sandbox ===========================\n");
			printf("The protected environment has been shut down.\n");
		}

		if (file_exist(sandbox_log, 0)) {
			sandbox_log_presence = 1;
			success = 1;
			if (!print_sandbox_log(sandbox_log))
				success = 0;

#if 0
			if (!success)
				exit(1);
#endif

		} else if (print_debug) {
			printf("--------------------------------------------------------------------------------\n");
		}

		free(sandbox_pids_file);

		if ((sandbox_log_presence) || (!success))
			return 1;
		else
			return 0;
	}
}

// vim:noexpandtab noai:cindent ai
