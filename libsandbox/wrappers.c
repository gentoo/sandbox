/*
 * wrappers.c
 *
 * Function wrapping functions.
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


#include "config.h"

/* Better way would be to only define _GNU_SOURCE when __GLIBC__ is defined,
 * but including features.h and then defining _GNU_SOURCE do not work */
#if defined(HAVE_RTLD_NEXT)
# define _GNU_SOURCE
#endif
#include <errno.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#include "localdecls.h"
#include "sbutil.h"
#include "libsandbox.h"
#include "wrappers.h"

#if !defined(BROKEN_RTLD_NEXT) && defined(HAVE_RTLD_NEXT)
# define USE_RTLD_NEXT
#endif

static void *libc_handle = NULL;

extern char sandbox_lib[SB_PATH_MAX];
extern int sandbox_on;

void *get_dlsym(const char *symname, const char *symver)
{
	void *symaddr = NULL;

	if (NULL == libc_handle) {
#if !defined(USE_RTLD_NEXT)
		libc_handle = dlopen(LIBC_VERSION, RTLD_LAZY);
		if (!libc_handle) {
			fprintf(stderr, "libsandbox:  Can't dlopen libc: %s\n",
				dlerror());
			exit(EXIT_FAILURE);
		}
#else
		libc_handle = RTLD_NEXT;
#endif
	}

	if (NULL == symver)
		symaddr = dlsym(libc_handle, symname);
	else
		symaddr = dlvsym(libc_handle, symname, symver);
	if (!symaddr) {
		fprintf(stderr, "libsandbox:  Can't resolve %s: %s\n",
			symname, dlerror());
		exit(EXIT_FAILURE);
	}

	return symaddr;
}

/*
 * Wrapper Functions
 */

#define chmod_decl(_name) \
\
extern int _name(const char *, mode_t); \
static int (*true_ ## _name) (const char *, mode_t) = NULL; \
\
int _name(const char *path, mode_t mode) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("chmod", path) { \
		check_dlsym(_name); \
		result = true_ ## _name(path, mode); \
	} \
\
	return result; \
}

#define chown_decl(_name) \
\
extern int _name(const char *, uid_t, gid_t); \
static int (*true_ ## _name) (const char *, uid_t, gid_t) = NULL; \
\
int _name(const char *path, uid_t owner, gid_t group) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("chown", path) { \
		check_dlsym(_name); \
		result = true_ ## _name(path, owner, group); \
	} \
\
	return result; \
}

#define creat_decl(_name) \
\
extern int _name(const char *, mode_t); \
/* static int (*true_ ## _name) (const char *, mode_t) = NULL; */ \
\
int _name(const char *pathname, mode_t mode) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("creat", pathname) { \
		check_dlsym(open_DEFAULT); \
		result = true_open_DEFAULT(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode); \
	} \
\
	return result; \
}

#define fopen_decl(_name) \
\
extern FILE *_name(const char *, const char *); \
static FILE * (*true_ ## _name) (const char *, const char *) = NULL; \
\
FILE *_name(const char *pathname, const char *mode) \
{ \
	FILE *result = NULL; \
\
	if FUNCTION_SANDBOX_SAFE_OPEN_CHAR("fopen", pathname, mode) { \
		check_dlsym(_name); \
		result = true_ ## _name(pathname, mode); \
	} \
\
	return result; \
}

#define lchown_decl(_name) \
\
extern int _name(const char *, uid_t, gid_t); \
static int (*true_ ## _name) (const char *, uid_t, gid_t) = NULL; \
\
int _name(const char *path, uid_t owner, gid_t group) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("lchown", path) { \
		check_dlsym(_name); \
		result = true_ ## _name(path, owner, group); \
	} \
\
	return result; \
}

#define link_decl(_name) \
\
extern int _name(const char *, const char *); \
static int (*true_ ## _name) (const char *, const char *) = NULL; \
\
int _name(const char *oldpath, const char *newpath) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("link", newpath) { \
		check_dlsym(_name); \
		result = true_ ## _name(oldpath, newpath); \
	} \
\
	return result; \
}

#define mkdir_decl(_name) \
\
extern int _name(const char *, mode_t); \
static int (*true_ ## _name) (const char *, mode_t) = NULL; \
\
int _name(const char *pathname, mode_t mode) \
{ \
	struct stat st; \
	int result = -1, my_errno = errno; \
	char canonic[SB_PATH_MAX]; \
\
	if (-1 == canonicalize(pathname, canonic)) \
		/* Path is too long to canonicalize, do not fail, but just let 
		 * the real function handle it (see bug #94630 and #21766). */ \
		if (ENAMETOOLONG != errno) \
			return -1; \
\
	/* XXX: Hack to prevent errors if the directory exist,
	 * and are not writable - we rather return EEXIST rather
	 * than failing */ \
	if (0 == lstat(canonic, &st)) { \
		errno = EEXIST; \
		return -1; \
	} \
	errno = my_errno; \
\
	if FUNCTION_SANDBOX_SAFE("mkdir", pathname) { \
		check_dlsym(_name); \
		result = true_ ## _name(pathname, mode); \
	} \
\
	return result; \
}

#define opendir_decl(_name) \
\
extern DIR *_name(const char *); \
static DIR * (*true_ ## _name) (const char *) = NULL; \
\
DIR *_name(const char *name) \
{ \
	DIR *result = NULL; \
\
	if FUNCTION_SANDBOX_SAFE("opendir", name) { \
		check_dlsym(_name); \
		result = true_ ## _name(name); \
	} \
\
	return result; \
}

#define mknod_decl(_name) \
\
extern int _name(const char *, mode_t, dev_t); \
static int (*true_ ## _name) (const char *, mode_t, dev_t) = NULL; \
\
int _name(const char *pathname, mode_t mode, dev_t dev) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("mknod", pathname) { \
		check_dlsym(_name); \
		result = true_ ## _name(pathname, mode, dev); \
	} \
\
	return result; \
}

#define __xmknod_decl(_name) \
\
extern int _name(int, const char *, __mode_t, __dev_t *); \
static int (*true_ ## _name) (int, const char *, __mode_t, __dev_t *) = NULL; \
\
int _name(int ver, const char *pathname, __mode_t mode, __dev_t *dev) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("mknod", pathname) { \
		check_dlsym(_name); \
		result = true_ ## _name(ver, pathname, mode, dev); \
	} \
\
	return result; \
}

#define mkfifo_decl(_name) \
\
extern int _name(const char *, mode_t); \
static int (*true_ ## _name) (const char *, mode_t) = NULL; \
\
int _name(const char *pathname, mode_t mode) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("mkfifo", pathname) { \
		check_dlsym(_name); \
		result = true_ ## _name(pathname, mode); \
	} \
\
	return result; \
}

#define access_decl(_name) \
\
extern int _name(const char *, int); \
static int (*true_ ## _name) (const char *, int) = NULL; \
\
int _name(const char *pathname, int mode) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE_ACCESS("access", pathname, mode) { \
		check_dlsym(_name); \
		result = true_ ## _name(pathname, mode); \
	} \
\
	return result; \
}

#define open_decl(_name) \
\
extern int _name(const char *, int, ...); \
static int (*true_ ## _name) (const char *, int, ...) = NULL; \
\
/* Eventually, there is a third parameter: it's mode_t mode */ \
int _name(const char *pathname, int flags, ...) \
{ \
	va_list ap; \
	int mode = 0; \
	int result = -1; \
\
	if (flags & O_CREAT) { \
		va_start(ap, flags); \
		mode = va_arg(ap, int); \
		va_end(ap); \
	} \
\
	if FUNCTION_SANDBOX_SAFE_OPEN_INT("open", pathname, flags) { \
		check_dlsym(_name); \
		if (flags & O_CREAT) \
			result = true_ ## _name(pathname, flags, mode); \
		else \
			result = true_ ## _name(pathname, flags); \
	} \
\
	return result; \
}

#define rename_decl(_name) \
\
extern int _name(const char *, const char *); \
static int (*true_ ## _name) (const char *, const char *) = NULL; \
\
int _name(const char *oldpath, const char *newpath) \
{ \
	int result = -1; \
\
	if (FUNCTION_SANDBOX_SAFE("rename", oldpath) && \
	    FUNCTION_SANDBOX_SAFE("rename", newpath)) { \
		check_dlsym(_name); \
		result = true_ ## _name(oldpath, newpath); \
	} \
\
	return result; \
}

#define rmdir_decl(_name) \
\
extern int _name(const char *); \
static int (*true_ ## _name) (const char *) = NULL; \
\
int _name(const char *pathname) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("rmdir", pathname) { \
		check_dlsym(_name); \
		result = true_ ## _name(pathname); \
	} \
\
	return result; \
}

#define symlink_decl(_name) \
\
extern int _name(const char *, const char *); \
static int (*true_ ## _name) (const char *, const char *) = NULL; \
\
int _name(const char *oldpath, const char *newpath) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("symlink", newpath) { \
		check_dlsym(_name); \
		result = true_ ## _name(oldpath, newpath); \
	} \
\
	return result; \
}

#define truncate_decl(_name) \
\
extern int _name(const char *, TRUNCATE_T); \
static int (*true_ ## _name) (const char *, TRUNCATE_T) = NULL; \
\
int _name(const char *path, TRUNCATE_T length) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("truncate", path) { \
		check_dlsym(_name); \
		result = true_ ## _name(path, length); \
	} \
\
	return result; \
}

#define unlink_decl(_name) \
\
extern int _name(const char *); \
static int (*true_ ## _name) (const char *) = NULL; \
\
int _name(const char *pathname) \
{ \
	int result = -1; \
	char canonic[SB_PATH_MAX]; \
\
	if (-1 == canonicalize(pathname, canonic)) \
		/* Path is too long to canonicalize, do not fail, but just let
		 * the real function handle it (see bug #94630 and #21766). */ \
		if (ENAMETOOLONG != errno) \
			return -1; \
\
	/* XXX: Hack to make sure sandboxed process cannot remove
	 * a device node, bug #79836. */ \
	if ((0 == strncmp(canonic, "/dev/null", 9)) || \
	    (0 == strncmp(canonic, "/dev/zero", 9))) { \
		errno = EACCES; \
		return result; \
	} \
\
	if FUNCTION_SANDBOX_SAFE("unlink", pathname) { \
		check_dlsym(_name); \
		result = true_ ## _name(pathname); \
	} \
\
	return result; \
}

#define getcwd_decl(_name) \
\
extern char *_name(char *, size_t); \
static char * (*true_ ## _name) (char *, size_t) = NULL; \
\
char *_name(char *buf, size_t size) \
{ \
	char *result = NULL; \
\
	/* Need to disable sandbox, as on non-linux libc's, opendir() is
	 * used by some getcwd() implementations and resolves to the sandbox
	 * opendir() wrapper, causing infinit recursion and finially crashes.
	 */ \
	sandbox_on = 0; \
	check_dlsym(_name); \
	result = true_ ## _name(buf, size); \
	sandbox_on = 1; \
\
	return result; \
}

#define creat64_decl(_name) \
\
extern int _name(const char *, __mode_t); \
/* static int (*true_ ## _name) (const char *, __mode_t) = NULL; */ \
\
int _name(const char *pathname, __mode_t mode) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("creat64", pathname) { \
		check_dlsym(open64_DEFAULT); \
		result = true_open64_DEFAULT(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode); \
	} \
\
	return result; \
}

#define fopen64_decl(_name) \
\
extern FILE *_name(const char *, const char *); \
static FILE * (*true_ ## _name) (const char *, const char *) = NULL; \
\
FILE *_name(const char *pathname, const char *mode) \
{ \
	FILE *result = NULL; \
\
	if FUNCTION_SANDBOX_SAFE_OPEN_CHAR("fopen64", pathname, mode) { \
		check_dlsym(_name); \
		result = true_ ## _name(pathname, mode); \
	} \
\
	return result; \
}

#define open64_decl(_name) \
\
extern int _name(const char *, int, ...); \
static int (*true_ ## _name) (const char *, int, ...) = NULL; \
\
/* Eventually, there is a third parameter: it's mode_t mode */ \
int _name(const char *pathname, int flags, ...) \
{ \
	va_list ap; \
	int mode = 0; \
	int result = -1; \
\
	if (flags & O_CREAT) { \
		va_start(ap, flags); \
		mode = va_arg(ap, int); \
		va_end(ap); \
	} \
\
	if FUNCTION_SANDBOX_SAFE_OPEN_INT("open64", pathname, flags) { \
		check_dlsym(_name); \
		if (flags & O_CREAT) \
			result = true_ ## _name(pathname, flags, mode); \
		else \
			result = true_ ## _name(pathname, flags); \
	} \
\
	return result; \
}

#define truncate64_decl(_name) \
\
extern int _name(const char *, __off64_t); \
static int (*true_ ## _name) (const char *, __off64_t) = NULL; \
\
int _name(const char *path, __off64_t length) \
{ \
	int result = -1; \
\
	if FUNCTION_SANDBOX_SAFE("truncate64", path) { \
		check_dlsym(_name); \
		result = true_ ## _name(path, length); \
	} \
\
	return result; \
}

/*
 * Exec Wrappers
 */

#define execve_decl(_name) \
\
extern int _name(const char *, char *const[], char *const[]); \
static int (*true_ ## _name) (const char *, char *const[], char *const[]) = NULL; \
\
int _name(const char *filename, char *const argv[], char *const envp[]) \
{ \
	int old_errno = errno; \
	int result = -1; \
	int count = 0; \
	int env_len = 0; \
	char **my_env = NULL; \
	int kill_env = 1; \
	/* We limit the size LD_PRELOAD can be here, but it should be enough */ \
	char tmp_str[SB_BUF_LEN]; \
\
	if FUNCTION_SANDBOX_SAFE("execve", filename) { \
		while (envp[count] != NULL) { \
			/* Check if we do not have to do anything */ \
			if (strstr(envp[count], LD_PRELOAD_EQ) == envp[count]) { \
				if (NULL != strstr(envp[count], sandbox_lib)) { \
					my_env = (char **)envp; \
					kill_env = 0; \
					goto end_loop; \
				} \
			} \
\
			/* If LD_PRELOAD is set and sandbox_lib not in it */ \
			if (((strstr(envp[count], LD_PRELOAD_EQ) == envp[count]) && \
			     (NULL == strstr(envp[count], sandbox_lib))) || \
			    /* Or  LD_PRELOAD is not set, and this is the last loop */ \
			    ((strstr(envp[count], LD_PRELOAD_EQ) != envp[count]) && \
			     (NULL == envp[count + 1]))) { \
				int i = 0; \
				int add_ldpreload = 0; \
				const int max_envp_len = strlen(envp[count]) + strlen(sandbox_lib) + 1; \
\
				/* Fail safe ... */ \
				if (max_envp_len > SB_BUF_LEN) { \
					fprintf(stderr, "libsandbox:  max_envp_len too big!\n"); \
					errno = ENOMEM; \
					return result; \
				} \
\
				/* Calculate envp size */ \
				my_env = (char **)envp; \
				do \
					env_len++; \
				while (NULL != *my_env++); \
\
				/* Should we add LD_PRELOAD ? */ \
				if (strstr(envp[count], LD_PRELOAD_EQ) != envp[count]) \
					add_ldpreload = 1; \
\
				my_env = (char **)xcalloc(env_len + add_ldpreload, sizeof(char *)); \
				if (NULL == my_env) \
					return result; \
				/* Copy envp to my_env */ \
				do \
					/* Leave a space for LD_PRELOAD if needed */ \
					my_env[i + add_ldpreload] = envp[i]; \
				while (NULL != envp[i++]); \
\
				/* Add 'LD_PRELOAD=' to the beginning of our new string */ \
				snprintf(tmp_str, max_envp_len, "%s%s", LD_PRELOAD_EQ, sandbox_lib); \
\
				/* LD_PRELOAD already have variables other than sandbox_lib,
				 * thus we have to add sandbox_lib seperated via a whitespace. */ \
				if (0 == add_ldpreload) { \
					snprintf((char *)(tmp_str + strlen(tmp_str)), \
						 max_envp_len - strlen(tmp_str) + 1, " %s", \
						 (char *)(envp[count] + strlen(LD_PRELOAD_EQ))); \
				} \
\
				/* Valid string? */ \
				tmp_str[max_envp_len] = '\0'; \
\
				/* Ok, replace my_env[count] with our version that contains
				 * sandbox_lib ... */ \
				if (1 == add_ldpreload) \
					/* We reserved a space for LD_PRELOAD above */ \
					my_env[0] = tmp_str; \
				else \
					my_env[count] = tmp_str; \
\
				goto end_loop; \
			} \
			count++; \
		} \
\
end_loop: \
		errno = old_errno; \
		check_dlsym(_name); \
		result = true_ ## _name(filename, argv, my_env); \
		old_errno = errno; \
\
		if (my_env && kill_env) \
			free(my_env); \
	} \
\
	errno = old_errno; \
\
	return result; \
}

#include "symbols.h"


int libsb_open(const char *pathname, int flags, ...)
{
	va_list ap;
	int mode = 0;
	int result = -1;

	if (flags & O_CREAT) {
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
	}

	check_dlsym(open_DEFAULT);
	if (flags & O_CREAT)
		result = true_open_DEFAULT(pathname, flags, mode);
	else
		result = true_open_DEFAULT(pathname, flags);

	return result;
}

char *libsb_getcwd(char *buf, size_t size)
{
	check_dlsym(getcwd_DEFAULT);
	
	return true_getcwd_DEFAULT(buf, size);
}

