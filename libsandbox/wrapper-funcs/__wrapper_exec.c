/*
 * common exec funcs.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO_FULL WRAPPER_ARGS_PROTO
#define WRAPPER_ARGS_FULL WRAPPER_ARGS
#ifndef WRAPPER_RET_TYPE
# define WRAPPER_RET_TYPE int
#endif
#ifndef WRAPPER_RET_DEFAULT
# define WRAPPER_RET_DEFAULT -1
#endif

extern WRAPPER_RET_TYPE EXTERN_NAME(WRAPPER_ARGS_PROTO);
static WRAPPER_RET_TYPE (*WRAPPER_TRUE_NAME)(WRAPPER_ARGS_PROTO) = NULL;

#ifndef SB_EXEC_COMMON
#define SB_EXEC_COMMON

/* Check to see if this a static ELF and if so, protect using trace mechanisms */
static void sb_check_exec(const char *filename, char *const argv[])
{
	int fd;
	unsigned char *elf;
	struct stat st;

	fd = open(filename, O_RDONLY);
	if (fd == -1)
		return;
	if (stat(filename, &st))
		goto out_fd;
	if (st.st_size < EI_NIDENT)
		goto out_fd;
	elf = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (elf == MAP_FAILED)
		goto out_fd;

	if (elf[EI_MAG0] != ELFMAG0 &&
	    elf[EI_MAG1] != ELFMAG1 &&
	    elf[EI_MAG2] != ELFMAG2 &&
	    elf[EI_MAG3] != ELFMAG3 &&
	    !(elf[EI_CLASS] != ELFCLASS32 ||
	      elf[EI_CLASS] != ELFCLASS64))
		goto out_mmap;

#define PARSE_ELF(n) \
({ \
	Elf##n##_Ehdr *ehdr = (void *)elf; \
	Elf##n##_Phdr *phdr = (void *)(elf + ehdr->e_phoff); \
	uint16_t p; \
	if (st.st_size < sizeof(*ehdr)) \
		goto out_mmap; \
	if (st.st_size < ehdr->e_phoff + ehdr->e_phentsize * ehdr->e_phnum) \
		goto out_mmap; \
	for (p = 0; p < ehdr->e_phnum; ++p) \
		if (phdr[p].p_type == PT_INTERP) \
			goto done; \
})
	if (elf[EI_CLASS] == ELFCLASS32)
		PARSE_ELF(32);
	else
		PARSE_ELF(64);

	/* We only support tracing of host personality atm */
	trace_main(filename, argv);

 done:

 out_mmap:
	munmap(elf, st.st_size);
 out_fd:
	close(fd);
}

static char **_sb_check_envp(char **envp, bool is_environ)
{
	char **my_env = NULL;
	char *entry;
	char *ld_preload = NULL;
	char *old_ld_preload = NULL;
	size_t count, ld_preload_eq_len;

	ld_preload_eq_len = strlen(LD_PRELOAD_EQ);
	str_list_for_each_item(envp, entry, count) {
		if (strncmp(entry, LD_PRELOAD_EQ, ld_preload_eq_len))
			continue;

		/* Check if we do not have to do anything */
		if (NULL != strstr(entry, sandbox_lib)) {
			/* Use the user's envp */
			return envp;
		} else {
			/* No need to continue (assuming the env is sane and does not
			 * include multiple entries for same var); we have to modify
			 * LD_PRELOAD to include our sandbox overrides
			 */
			old_ld_preload = entry;
			break;
		}
	}

	/* Ok, we need to create our own envp, as we need to add LD_PRELOAD,
	 * and we should not touch the user's envp.  First we add LD_PRELOAD,
	 * and just all the rest. */
	count = ld_preload_eq_len + (strlen(sandbox_lib) + 1) +
		(old_ld_preload ? strlen(old_ld_preload) - ld_preload_eq_len + 1 : 0);
	ld_preload = xmalloc(count * sizeof(char));
	snprintf(ld_preload, count, "%s%s%s%s", LD_PRELOAD_EQ, sandbox_lib,
		 (old_ld_preload) ? " " : "",
		 (old_ld_preload) ? old_ld_preload + ld_preload_eq_len : "");

	if (!is_environ) {
		str_list_add_item(my_env, ld_preload, error);

		str_list_for_each_item(envp, entry, count) {
			if (strncmp(entry, LD_PRELOAD_EQ, ld_preload_eq_len)) {
				str_list_add_item(my_env, entry, error);
				continue;
			}
		}
	} else
		putenv(ld_preload);

 error:
	return my_env;
}
static char **sb_check_envp(char **envp)
{
	return _sb_check_envp(envp, false);
}
static void sb_check_environ(void)
{
	_sb_check_envp(environ, true);
}

static void sb_cleanup_envp(char **envp)
{
	/* We assume the LD_PRELOAD is the first entry */
	free(envp[0]);

	/* We do not use str_list_free(), as we did not allocate the
	 * entries except for LD_PRELOAD.  All the other entries are
	 * pointers to existing envp memory.
	 */
	free(envp);
}

#endif

attribute_hidden
WRAPPER_RET_TYPE SB_HIDDEN_FUNC(WRAPPER_NAME)(WRAPPER_ARGS_PROTO_FULL)
{
	check_dlsym(WRAPPER_TRUE_NAME, WRAPPER_SYMNAME, WRAPPER_SYMVER);
	return WRAPPER_TRUE_NAME(WRAPPER_ARGS_FULL);
}

#ifndef EXEC_ARGS
# define EXEC_ARGS WRAPPER_ARGS_FULL
#endif
WRAPPER_RET_TYPE WRAPPER_NAME(WRAPPER_ARGS_PROTO)
{
	WRAPPER_RET_TYPE result = WRAPPER_RET_DEFAULT;

#ifdef EXEC_MY_ENV
	char **my_env = (char **)envp;
#endif

	/* The C library may implement some exec funcs by calling other
	 * exec funcs.  So we might get a little sandbox recursion going
	 * on.  But this shouldn't cause a problem now should it ?
	 */
#ifdef EXEC_RECUR_CHECK
	static __thread size_t recursive = 0;

	if (recursive++)
		goto do_exec_only;
#endif

	save_errno();

#ifndef EXEC_NO_FILE
	const char *check_path = path;
	char *mem1 = NULL, *mem2 = NULL;
# ifndef EXEC_NO_PATH
	/* Some exec funcs always operate on full paths, while others
	 * will search $PATH if the specified name lacks a slash.
	 */
	char *envpath = getenv("PATH");
	if (!strchr(check_path, '/') && envpath) {
		size_t len_path = strlen(check_path);
		char *p, *pp;
		check_path = NULL;
		pp = envpath = mem1 = xstrdup(envpath);
		p = strtok_r(envpath, ":", &pp);
		while (p) {
			mem2 = xrealloc(mem2, strlen(p) + 1 + len_path + 1);
			sprintf(mem2, "%s/%s", p, path);
			if (access(mem2, R_OK) == 0) {
				check_path = mem2;
				break;
			}
			p = strtok_r(NULL, ":", &pp);
		}
	}

# endif
	if (check_path) {
		if (!SB_SAFE(check_path))
			goto done;

		sb_check_exec(check_path, argv);
	}
#endif

#ifdef EXEC_MY_ENV
	my_env = sb_check_envp(my_env);
#else
	sb_check_environ();
#endif

	restore_errno();
#ifdef EXEC_RECUR_CHECK
 do_exec_only:
#endif
	result = SB_HIDDEN_FUNC(WRAPPER_NAME)(EXEC_ARGS);

#ifdef EXEC_MY_ENV
	if (my_env != envp)
		sb_cleanup_envp(my_env);
#endif

#ifdef EXEC_RECUR_CHECK
	--recursive;
#endif

#ifndef EXEC_NO_FILE
 done:
	free(mem1);
	free(mem2);
#endif
	return result;
}

#undef EXEC_ARGS
#undef EXEC_MY_ENV
#undef EXEC_NO_PATH
#undef WRAPPER_ARGS_FULL
#undef WRAPPER_ARGS_PROTO_FULL
#undef WRAPPER_SAFE_POST_EXPAND
