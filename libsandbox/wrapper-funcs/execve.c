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
	char **my_env = NULL;
	char *entry;
	char *ld_preload = NULL;
	char *old_ld_preload = NULL;
	int old_errno = errno;
	int result = -1;
	int count;

	if (!FUNCTION_SANDBOX_SAFE("execve", filename))
		return result;

	str_list_for_each_item(envp, entry, count) {
		if (strstr(entry, LD_PRELOAD_EQ) != entry)
			continue;

		/* Check if we do not have to do anything */
		if (NULL != strstr(entry, sandbox_lib)) {
			/* Use the user's envp */
			my_env = (char **)envp;
			goto do_execve;
		} else {
			old_ld_preload = entry;
			/* No need to continue, we have to modify LD_PRELOAD */
			break;
		}
	}

	/* Ok, we need to create our own envp, as we need to add LD_PRELOAD,
	 * and we should not touch the user's envp.  First we add LD_PRELOAD,
	 * and just all the rest. */
	count = strlen(LD_PRELOAD_EQ) + strlen(sandbox_lib) + 1;
	if (NULL != old_ld_preload)
		count += strlen(old_ld_preload) - strlen(LD_PRELOAD_EQ) + 1;
	ld_preload = xmalloc(count * sizeof(char));
	if (NULL == ld_preload)
		goto error;
	snprintf(ld_preload, count, "%s%s%s%s", LD_PRELOAD_EQ, sandbox_lib,
		 (old_ld_preload) ? " " : "",
		 (old_ld_preload) ? old_ld_preload + strlen(LD_PRELOAD_EQ) : "");
	str_list_add_item(my_env, ld_preload, error);

	str_list_for_each_item(envp, entry, count) {
		if (strstr(entry, LD_PRELOAD_EQ) != entry) {
			str_list_add_item(my_env, entry, error);
			continue;
		}
	}

do_execve:
	errno = old_errno;
	check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME,
		    WRAPPER_SYMVER);
	result = WRAPPER_TRUE_NAME(filename, argv, my_env);

	if ((NULL != my_env) && (my_env != envp))
		/* We do not use str_list_free(), as we did not allocate the
		 * entries except for LD_PRELOAD. */
		free(my_env);
	if (NULL != ld_preload)
		free(ld_preload);

	return result;

error:
	if ((NULL != my_env) && (my_env != envp))
		/* We do not use str_list_free(), as we did not allocate the
		 * entries except for LD_PRELOAD. */
		free(my_env);
	if (NULL != ld_preload)
		free(ld_preload);

	return -1;
}

