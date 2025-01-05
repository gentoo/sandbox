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

/* Check to see if we can run this program in-process.  If not, try to fall back
 * tracing it out-of-process via some trace mechanisms (e.g. ptrace).
 */
static bool sb_check_exec(const char *filename, char *const argv[])
{
	int fd;
	unsigned char *elf;
	struct stat st;
	bool do_trace = false;
	bool run_in_process = true;
	sandbox_method_t method = get_sandbox_method();

	if (unlikely(method == SANDBOX_METHOD_PRELOAD))
		return true;

	fd = sb_unwrapped_open_DEFAULT(filename, O_RDONLY|O_CLOEXEC, 0);
	if (fd == -1)
		return true;
	if (fstat(fd, &st))
		goto out_fd;
	if (st.st_size < sizeof(Elf64_Ehdr))
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

	/* If we are non-root but attempt to execute a set*id program,
	 * our LD_PRELOAD trick won't work.  So skip the static check.
	 * This might break some apps, but it shouldn't, and is better
	 * than doing nothing since it might mean `mount` or `umount`
	 * won't get caught if/when they modify things. #442172
	 *
	 * Only other option is to code a set*id sandbox helper that
	 * gains root just to preload libsandbox.so.  That unfortunately
	 * could easily open up people to root vulns.
	 */
	if (st.st_mode & (S_ISUID | S_ISGID))
		if (getuid() != 0)
			run_in_process = false;

	/* We also need to ptrace programs that interpose their own allocator.
	 * https://crbug.com/586444
	 */
	if (run_in_process) {
		static const char * const libc_alloc_syms[] = {
			"__libc_calloc",
			"__libc_free",
			"__libc_malloc",
			"__libc_realloc",
			"__malloc_hook",
			"__realloc_hook",
			"__free_hook",
			"__memalign_hook",
			"__malloc_initialize_hook",
		};
#define PARSE_ELF(n) \
({ \
	Elf##n##_Ehdr *ehdr = (void *)elf; \
	Elf##n##_Phdr *phdr = (void *)(elf + ehdr->e_phoff); \
	Elf##n##_Addr vaddr, filesz, vsym = 0, vstr = 0, vhash = 0, vgnuhash = 0; \
	Elf##n##_Off offset, symoff = 0, stroff = 0, hashoff = 0, gnuhashoff = 0; \
	Elf##n##_Dyn *dyn; \
	Elf##n##_Sym *sym, *symend; \
	uint##n##_t ent_size = 0, str_size = 0; \
	bool dynamic = false; \
	size_t i; \
	\
	if (st.st_size < ehdr->e_phoff + ehdr->e_phentsize * ehdr->e_phnum) \
		goto out_mmap; \
	\
	/* First gather the tags we care about. */ \
	for (i = 0; i < ehdr->e_phnum; ++i) { \
		switch (phdr[i].p_type) { \
		case PT_INTERP: dynamic = true; break; \
		case PT_DYNAMIC: \
			dyn = (void *)(elf + phdr[i].p_offset); \
			while (dyn->d_tag != DT_NULL) { \
				switch (dyn->d_tag) { \
				case DT_SYMTAB:      vsym = dyn->d_un.d_val; break; \
				case DT_SYMENT:      ent_size = dyn->d_un.d_val; break; \
				case DT_STRTAB:      vstr = dyn->d_un.d_val; break; \
				case DT_STRSZ:       str_size = dyn->d_un.d_val; break; \
				case DT_HASH:        vhash = dyn->d_un.d_val; break; \
				case DT_GNU_HASH:    vgnuhash = dyn->d_un.d_val; break; \
				} \
				++dyn; \
			} \
			break; \
		} \
	} \
	\
	if (dynamic && vsym && ent_size && vstr && str_size) { \
		/* Figure out where in the file these tables live. */ \
		for (i = 0; i < ehdr->e_phnum; ++i) { \
			vaddr = phdr[i].p_vaddr; \
			filesz = phdr[i].p_filesz; \
			offset = phdr[i].p_offset; \
			if (vsym >= vaddr && vsym < vaddr + filesz) \
				symoff = offset + (vsym - vaddr); \
			if (vstr >= vaddr && vstr < vaddr + filesz) \
				stroff = offset + (vstr - vaddr); \
			if (vhash >= vaddr && vhash < vaddr + filesz) \
				hashoff = offset + (vhash - vaddr); \
			if (vgnuhash >= vaddr && vgnuhash < vaddr + filesz) \
				gnuhashoff = offset + (vgnuhash - vaddr); \
		} \
		\
		/* Finally walk the symbol table.  This should generally be fast as \
		 * we only look at exported symbols, and the vast majority of exes \
		 * out there do not export any symbols at all. \
		 */ \
		if (symoff && stroff) { \
			/* Nowhere is the # of symbols recorded, or the size of the symbol \
			 * table.  Instead, we do what glibc does: use the gnu or sysv hash \
			 * table if it exists, else assume that the string table always directly \
			 * follows the symbol table.  This seems like a poor assumption to \
			 * make, but glibc has gotten by this long.  See determine_info in \
			 * glibc's elf/dl-addr.c. \
			 * \
			 * We don't sanity check the ranges here as you aren't executing \
			 * corrupt programs in the sandbox. \
			 */ \
			sym = (void *)(elf + symoff); \
			if (vgnuhash) { \
				uint32_t *hash32 = (void *)(elf + gnuhashoff); \
				/* use glibc's elf/dl-lookup.c:_dl_setup_hash() as a reference */ \
				/*   DT_GNU_HASH header: */ \
				uint32_t nbuckets = *hash32++; \
				uint32_t symbias = *hash32++; \
				uint32_t bitmask_nwords = *hash32++; \
				hash32++; /* gnu_shift */ \
				hash32 += n / 32 * bitmask_nwords; /* gnu_bitmask */ \
				uint32_t *gnu_buckets = hash32; \
				hash32 += nbuckets; \
				uint32_t *gnu_chain_zero = hash32 - symbias; \
				\
				uint32_t bucket; \
				\
				for (bucket = 0; bucket < nbuckets; bucket++) { \
					uint32_t symndx = gnu_buckets[bucket]; \
					if (symndx != 0) { \
						const uint32_t *hasharr = &gnu_chain_zero[symndx]; \
						do { \
							Elf##n##_Sym * s = &sym[symndx]; \
							\
							/* keep in sync with 'vhash' case */ \
							char *symname = (void *)(elf + stroff + s->st_name); \
							if (ELF##n##_ST_VISIBILITY(s->st_other) == STV_DEFAULT && \
							    s->st_shndx != SHN_UNDEF && s->st_shndx < SHN_LORESERVE && \
							    s->st_name && \
							    /* Minor optimization to avoid strcmp. */ \
							    symname[0] == '_' && symname[1] == '_') { \
								/* Blacklist internal C library symbols. */ \
								for (i = 0; i < ARRAY_SIZE(libc_alloc_syms); ++i) \
									if (!strcmp(symname, libc_alloc_syms[i])) { \
										run_in_process = false; \
										goto use_trace; \
									} \
							} \
							++symndx; \
						} while ((*hasharr++ & 1u) == 0); \
					} \
				} \
			} else { \
				if (vhash) { \
					/* Hash entries are always 32-bits. */ \
					uint32_t *hashes = (void *)(elf + hashoff); \
					symend = sym + hashes[1]; \
				} else \
					symend = (void *)(elf + stroff); \
				\
				while (sym < symend) { \
					/* keep insync with 'vgnuhash' case */ \
					char *symname = (void *)(elf + stroff + sym->st_name); \
					if (ELF##n##_ST_VISIBILITY(sym->st_other) == STV_DEFAULT && \
					    sym->st_shndx != SHN_UNDEF && sym->st_shndx < SHN_LORESERVE && \
					    sym->st_name && \
					    /* Minor optimization to avoid strcmp. */ \
					    symname[0] == '_' && symname[1] == '_') { \
						/* Blacklist internal C library symbols. */ \
						for (i = 0; i < ARRAY_SIZE(libc_alloc_syms); ++i) \
							if (!strcmp(symname, libc_alloc_syms[i])) { \
								run_in_process = false; \
								goto use_trace; \
							} \
					} \
					++sym; \
				} \
			} \
		} \
		\
	} \
	\
	if (dynamic) \
		goto done; \
})
		if (elf[EI_CLASS] == ELFCLASS32)
			PARSE_ELF(32);
		else
			PARSE_ELF(64);
#undef PARSE_ELF
	}

 use_trace:
	do_trace = trace_possible(filename, argv, elf);
	/* Now that we're done with stuff, clean up before forking */

 done:

 out_mmap:
	munmap(elf, st.st_size);
 out_fd:
	close(fd);

	if (do_trace) {
		sb_debug_dyn("tracing: %s\n", filename);
		trace_main();
	}

	return run_in_process;
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
	bool run_in_process = true;

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

		run_in_process = sb_check_exec(check_path, argv);
	}
#endif

#ifdef EXEC_MY_ENV
	struct sb_envp_ctx ec = sb_new_envp((char**)envp, run_in_process);
	char **my_env = ec.sb_envp;
#else
	struct sb_envp_ctx ec = sb_new_envp(environ, run_in_process);
	environ = ec.sb_envp;
#endif

	restore_errno();
#ifdef EXEC_RECUR_CHECK
 do_exec_only:
#endif
	result = SB_HIDDEN_FUNC(WRAPPER_NAME)(EXEC_ARGS);

#ifndef EXEC_MY_ENV
	/* https://bugs.gentoo.org/669702: maintain illusion
	 or unmodified 'environ'. */
	environ = ec.orig_envp;
#endif
	sb_free_envp(&ec);

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
