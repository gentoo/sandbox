/*
 * execve.c
 *
 * execve() wrapper.
 *
 * Copyright 1999-2006 Gentoo Foundation
 *
 *
 *      This program is free software; you can redistribute it and/or modify it
 *      under the terms of the GNU General Public License as published by the
 *      Free Software Foundation version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 *
 * $Header$
 */


extern int EXTERN_NAME(const char *, char *const[], char *const[]);
static int (*WRAPPER_TRUE_NAME) (const char *, char *const[], char *const[]) = NULL;

int WRAPPER_NAME(const char *filename, char *const argv[], char *const envp[])
{
	int old_errno = errno;
	int result = -1;
	int count = 0;
	int env_len = 0;
	char **my_env = NULL;
	int kill_env = 1;
	/* We limit the size LD_PRELOAD can be here, but it should be enough */
	char tmp_str[SB_BUF_LEN];

	if FUNCTION_SANDBOX_SAFE("execve", filename) {
		while (envp[count] != NULL) {
			/* Check if we do not have to do anything */
			if (strstr(envp[count], LD_PRELOAD_EQ) == envp[count]) {
				if (NULL != strstr(envp[count], sandbox_lib)) {
					my_env = (char **)envp;
					kill_env = 0;
					goto end_loop;
				}
			}

			/* If LD_PRELOAD is set and sandbox_lib not in it */
			if (((strstr(envp[count], LD_PRELOAD_EQ) == envp[count]) &&
			     (NULL == strstr(envp[count], sandbox_lib))) ||
			    /* Or  LD_PRELOAD is not set, and this is the last loop */
			    ((strstr(envp[count], LD_PRELOAD_EQ) != envp[count]) &&
			     (NULL == envp[count + 1]))) {
				int i = 0;
				int add_ldpreload = 0;
				const int max_envp_len = strlen(envp[count]) + strlen(sandbox_lib) + 1;

				/* Fail safe ... */
				if (max_envp_len > SB_BUF_LEN) {
					fprintf(stderr, "libsandbox:  max_envp_len too big!\n");
					errno = ENOMEM;
					return result;
				}

				/* Calculate envp size */
				my_env = (char **)envp;
				do
					env_len++;
				while (NULL != *my_env++);

				/* Should we add LD_PRELOAD ? */
				if (strstr(envp[count], LD_PRELOAD_EQ) != envp[count])
					add_ldpreload = 1;

				my_env = (char **)xcalloc(env_len + add_ldpreload, sizeof(char *));
				if (NULL == my_env)
					return result;
				/* Copy envp to my_env */
				do
					/* Leave a space for LD_PRELOAD if needed */
					my_env[i + add_ldpreload] = envp[i];
				while (NULL != envp[i++]);

				/* Add 'LD_PRELOAD=' to the beginning of our new string */
				snprintf(tmp_str, max_envp_len, "%s%s", LD_PRELOAD_EQ, sandbox_lib);

				/* LD_PRELOAD already have variables other than sandbox_lib,
				 * thus we have to add sandbox_lib seperated via a whitespace. */
				if (0 == add_ldpreload) {
					snprintf((char *)(tmp_str + strlen(tmp_str)),
						 max_envp_len - strlen(tmp_str) + 1, " %s",
						 (char *)(envp[count] + strlen(LD_PRELOAD_EQ)));
				}

				/* Valid string? */
				tmp_str[max_envp_len] = '\0';

				/* Ok, replace my_env[count] with our version that contains
				 * sandbox_lib ... */
				if (1 == add_ldpreload)
					/* We reserved a space for LD_PRELOAD above */
					my_env[0] = tmp_str;
				else
					my_env[count] = tmp_str;

				goto end_loop;
			}
			count++;
		}

end_loop:
		errno = old_errno;
		check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
			    WRAPPER_SYMVER);
		result = WRAPPER_TRUE_NAME(filename, argv, my_env);
		old_errno = errno;

		if (my_env && kill_env)
			free(my_env);
	}

	errno = old_errno;

	return result;
}

