/*
 * environ.c
 *
 * Environment setup and related functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"
#include "sandbox.h"

/* Replace '${FOO}' style strings in passed data with the value of named
 * environment variable. */
static char *subst_env_vars(rc_dynbuf_t *env_data)
{
	rc_dynbuf_t *new_data = NULL;
	char *tmp_ptr, *tmp_data = NULL;
	char *var_start, *var_stop;

	new_data = rc_dynbuf_new();

	tmp_data = rc_dynbuf_read_line(env_data);
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
		if (-1 == rc_dynbuf_sprintf(new_data, "%s%s",
					  tmp_ptr ? tmp_ptr : "",
					  env ? env : ""))
			goto error;

		/* Move tmp_ptr past the '}' of the env var */
		tmp_ptr = var_stop + 1;
	}

	if (0 != strlen(tmp_ptr))
		if (-1 == rc_dynbuf_write(new_data, tmp_ptr, strlen(tmp_ptr)))
			goto error;

	free(tmp_data);

	tmp_data = rc_dynbuf_read_line(new_data);
	if (NULL == tmp_data)
		goto error;

	rc_dynbuf_free(new_data);

	return tmp_data;

error:
	rc_dynbuf_free(new_data);
	if (NULL != tmp_data)
		free(tmp_data);

	return NULL;
}

static char *sb_conf_file(void)
{
	static char *conf_file;
	if (!conf_file)
		conf_file = get_sandbox_conf();
	return conf_file;
}

/* Get passed variable from sandbox.conf, and set it in the environment. */
static void setup_cfg_var(const char *env_var)
{
	char *config;

	/* We check if the variable is set in the environment, and if not, we
	 * get it from sandbox.conf, and if they exist, we just add them to the
	 * environment if not already present. */
	config = rc_get_cnf_entry(sb_conf_file(), env_var, NULL);
	if (NULL != config) {
		setenv(ENV_SANDBOX_VERBOSE, config, 0);
		free(config);
	}
}

/* Get passed access variable from sandbox.conf for sandbox.d/, and set it in
 * the environment. */
static int setup_access_var(const char *access_var)
{
	rc_dynbuf_t *env_data;
  	int count = 0;
	char *config;
	char **confd_files = NULL;
	bool use_confd = true;

	env_data = rc_dynbuf_new();

	/* Now get the defaults for the access variable from sandbox.conf.
	 * These do not get overridden via the environment. */
	config = rc_get_cnf_entry(sb_conf_file(), access_var, ":");
	if (NULL != config) {
		if (-1 == rc_dynbuf_write(env_data, config, strlen(config)))
			goto error;
		free(config);
		config = NULL;
	}
	/* Append whatever might be already set.  If anything is set, we do
	 * not process the sandbox.d/ files for this variable. */
	if (NULL != getenv(access_var)) {
		use_confd = false;
		if (-1 == rc_dynbuf_sprintf(env_data, env_data->wr_index ? ":%s" : "%s",
					  getenv(access_var)))
			goto error;
	}

	if (!use_confd)
		goto done;

	/* Now scan the files in sandbox.d/ if the access variable was not
	 * alreay set. */
	confd_files = rc_ls_dir(SANDBOX_CONFD_DIR, false, true);
	if (NULL != confd_files) {
		while (NULL != confd_files[count]) {
			config = rc_get_cnf_entry(confd_files[count], access_var, ":");
			if (NULL != config) {
				if (-1 == rc_dynbuf_sprintf(env_data,
							  env_data->wr_index ? ":%s" : "%s",
							  config))
					goto error;
				free(config);
				config = NULL;
			}
			count++;
		}

		str_list_free(confd_files);
		confd_files = NULL;
	}

done:
	if (env_data->wr_index > 0) {
	  	char *subst;

		subst = subst_env_vars(env_data);
		if (NULL == subst)
			goto error;

		setenv(access_var, subst, 1);
		free(subst);
	}

	rc_dynbuf_free(env_data);

	return 0;

error:
	rc_dynbuf_free(env_data);
	if (NULL != config)
		free(config);
	if (NULL != confd_files)
		str_list_free(confd_files);

	return -1;
}

/* Initialize all config and access variables, and set them in the
 * environment. */
static int setup_cfg_vars(struct sandbox_info_t *sandbox_info)
{
	setup_cfg_var(ENV_SANDBOX_VERBOSE);
	setup_cfg_var(ENV_SANDBOX_DEBUG);
	setup_cfg_var(ENV_SANDBOX_BEEP);
	setup_cfg_var(ENV_NOCOLOR);

	if (-1 == setup_access_var(ENV_SANDBOX_DENY))
		return -1;
	if (NULL == getenv(ENV_SANDBOX_DENY))
		setenv(ENV_SANDBOX_DENY, LD_PRELOAD_FILE, 1);

	if (-1 == setup_access_var(ENV_SANDBOX_READ))
		return -1;
	if (NULL == getenv(ENV_SANDBOX_READ))
		setenv(ENV_SANDBOX_READ, "/", 1);

	if (-1 == setup_access_var(ENV_SANDBOX_WRITE))
		return -1;
	if ((NULL == getenv(ENV_SANDBOX_WRITE)) &&
	    (NULL != sandbox_info->work_dir))
		setenv(ENV_SANDBOX_WRITE, sandbox_info->work_dir, 1);

	if (-1 == setup_access_var(ENV_SANDBOX_PREDICT))
		return -1;
	if ((NULL == getenv(ENV_SANDBOX_PREDICT)) &&
	    (NULL != sandbox_info->home_dir))
		setenv(ENV_SANDBOX_PREDICT, sandbox_info->home_dir, 1);

	return 0;
}

static void sb_setenv(char ***envp, const char *name, const char *val)
{
	char *tmp_string;

	/* strlen(name) + strlen(val) + '=' + '\0' */
	tmp_string = xmalloc((strlen(name) + strlen(val) + 2) * sizeof(char));

	snprintf(tmp_string, strlen(name) + strlen(val) + 2,
		 "%s=%s", name, val);

	str_list_add_item((*envp), tmp_string, error);

	return;

 error:
	sb_pwarn("out of memory");
	exit(EXIT_FAILURE);
}

/* We setup the environment child side only to prevent issues with
 * setting LD_PRELOAD parent side */
char **setup_environ(struct sandbox_info_t *sandbox_info, bool interactive)
{
	int have_ld_preload = 0;

	char **new_environ = NULL;
	char **env_ptr;
	char *ld_preload_envvar = NULL;
	char *orig_ld_preload_envvar = NULL;
	char sb_pid[64];

	if (-1 == setup_cfg_vars(sandbox_info))
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

	orig_ld_preload_envvar = getenv(ENV_LD_PRELOAD);
	if (orig_ld_preload_envvar) {
		if (!strstr(orig_ld_preload_envvar, sandbox_info->sandbox_lib)) {
			have_ld_preload = 1;
			ld_preload_envvar = xcalloc(strlen(orig_ld_preload_envvar) +
					strlen(sandbox_info->sandbox_lib) + 2,
					sizeof(char));
			snprintf(ld_preload_envvar, strlen(orig_ld_preload_envvar) +
					strlen(sandbox_info->sandbox_lib) + 2, "%s %s",
					sandbox_info->sandbox_lib, orig_ld_preload_envvar);
		} else {
			have_ld_preload = 2;
			ld_preload_envvar = NULL;
		}
	} else
		ld_preload_envvar = xstrdup(sandbox_info->sandbox_lib);
	/* Do not unset this, as strange things might happen */
	/* unsetenv(ENV_LD_PRELOAD); */

	snprintf(sb_pid, sizeof(sb_pid), "%i", getpid());

	/* First add our new variables to the beginning - this is due to some
	 * weirdness that I cannot remember */
	sb_setenv(&new_environ, ENV_SANDBOX_ON, "1");
	sb_setenv(&new_environ, ENV_SANDBOX_PID, sb_pid);
	sb_setenv(&new_environ, ENV_SANDBOX_LIB, sandbox_info->sandbox_lib);
	sb_setenv(&new_environ, ENV_SANDBOX_BASHRC, sandbox_info->sandbox_rc);
	sb_setenv(&new_environ, ENV_SANDBOX_LOG, sandbox_info->sandbox_log);
	sb_setenv(&new_environ, ENV_SANDBOX_DEBUG_LOG,
			sandbox_info->sandbox_debug_log);
	/* Is this an interactive session? */
	if (interactive)
		sb_setenv(&new_environ, ENV_SANDBOX_INTRACTV, "1");
	/* Just set the these if not already set so that is_env_on() work */
	if (!getenv(ENV_SANDBOX_VERBOSE))
		sb_setenv(&new_environ, ENV_SANDBOX_VERBOSE, "1");
	if (!getenv(ENV_SANDBOX_DEBUG))
		sb_setenv(&new_environ, ENV_SANDBOX_DEBUG, "0");
	if (!getenv(ENV_NOCOLOR))
		sb_setenv(&new_environ, ENV_NOCOLOR, "no");
	/* If LD_PRELOAD was not set, set it here, else do it below */
	if (!have_ld_preload)
		sb_setenv(&new_environ, ENV_LD_PRELOAD, ld_preload_envvar);

	/* Make sure our bashrc gets preference */
	sb_setenv(&new_environ, ENV_BASH_ENV, sandbox_info->sandbox_rc);

	/* This one should NEVER be set in ebuilds, as it is the one
	 * private thing libsandbox.so use to test if the sandbox
	 * should be active for this pid, or not.
	 *
	 * azarah (3 Aug 2002)
	 */

	sb_setenv(&new_environ, ENV_SANDBOX_ACTIVE, SANDBOX_ACTIVE);

	/* Now add the rest */
	env_ptr = environ;
	while (NULL != *env_ptr) {
		if ((1 == have_ld_preload) &&
		    (strstr(*env_ptr, LD_PRELOAD_EQ) == *env_ptr))
			/* If LD_PRELOAD was set, and this is it in the original
			 * environment, replace it with our new copy */
			/* XXX: The following works as it just add whatever as
			 *      the last variable to nev_environ */
			sb_setenv(&new_environ, ENV_LD_PRELOAD,
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
