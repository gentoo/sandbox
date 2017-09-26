/*
 * libsandbox.c
 *
 * Main libsandbox related functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 *
 *  Partly Copyright (C) 1998-9 Pancrazio `Ezio' de Mauro <p@demauro.net>,
 *  as some of the InstallWatch code was used.
 */

#include "headers.h"
#include "sbutil.h"
#include "libsandbox.h"
#include "wrappers.h"
#include "sb_nr.h"

#define LOG_VERSION			"1.0"
#define LOG_STRING			"VERSION " LOG_VERSION "\n"
#define LOG_FMT_FUNC			"FORMAT: F - Function called\n"
#define LOG_FMT_ACCESS			"FORMAT: S - Access Status\n"
#define LOG_FMT_PATH			"FORMAT: P - Path as passed to function\n"
#define LOG_FMT_APATH			"FORMAT: A - Absolute Path (not canonical)\n"
#define LOG_FMT_RPATH			"FORMAT: R - Canonical Path\n"
#define LOG_FMT_CMDLINE			"FORMAT: C - Command Line\n"

char sandbox_lib[SB_PATH_MAX];

typedef struct {
	bool show_access_violation, on, active, testing, verbose, debug;
	char *ld_library_path;
	char **prefixes[5];
	int num_prefixes[5];
#define             deny_prefixes     prefixes[0]
#define         num_deny_prefixes num_prefixes[0]
#define             read_prefixes     prefixes[1]
#define         num_read_prefixes num_prefixes[1]
#define            write_prefixes     prefixes[2]
#define        num_write_prefixes num_prefixes[2]
#define          predict_prefixes     prefixes[3]
#define      num_predict_prefixes num_prefixes[3]
#define     write_denied_prefixes     prefixes[4]
#define num_write_denied_prefixes num_prefixes[4]
#define MAX_DYN_PREFIXES 4 /* the first 4 are dynamic */
} sbcontext_t;
static sbcontext_t sbcontext;

static char *cached_env_vars[MAX_DYN_PREFIXES];
static char log_path[SB_PATH_MAX];
static char debug_log_path[SB_PATH_MAX];
static char message_path[SB_PATH_MAX];
bool sandbox_on = true;
static bool sb_init = false;
static bool sb_env_init = false;
int (*sbio_open)(const char *, int, mode_t) = sb_unwrapped_open;
FILE *(*sbio_popen)(const char *, const char *) = sb_unwrapped_popen;

static char *resolve_path(const char *, int);
static int check_prefixes(char **, int, const char *);
static void clean_env_entries(char ***, int *);
static void sb_process_env_settings(void);

const char *sbio_message_path;
const char sbio_fallback_path[] = "/dev/tty";

/* We need to initialize these vars before main().  This is to handle programs
 * (like `env`) that will clear the environment before making any syscalls
 * other than execve().  At that point, trying to get the settings is too late.
 * However, we might still need to init the env vars in the syscall wrapper for
 * programs that have their own constructors.  #404013
 */
__attribute__((constructor))
void libsb_init(void)
{
	if (sb_env_init)
		/* Ah, we already saw a syscall */
		return;
	sb_env_init = true;

	/* Get the path and name to this library */
	get_sandbox_lib(sandbox_lib);

	get_sandbox_log(log_path, NULL);
	get_sandbox_debug_log(debug_log_path, NULL);
	get_sandbox_message_path(message_path);
	sbio_message_path = message_path;

	memset(&sbcontext, 0x00, sizeof(sbcontext));
	sbcontext.show_access_violation = true;

	sb_process_env_settings();
	is_sandbox_on();
	sbcontext.verbose = is_env_on(ENV_SANDBOX_VERBOSE);
	sbcontext.debug = is_env_on(ENV_SANDBOX_DEBUG);
	sbcontext.testing = is_env_on(ENV_SANDBOX_TESTING);
	if (sbcontext.testing) {
		const char *ldpath = getenv("LD_LIBRARY_PATH");
		if (ldpath)
			sbcontext.ld_library_path = xstrdup(ldpath);
	}
}

/* resolve_dirfd_path - get the path relative to a dirfd
 *
 * return value:
 * -1 - error!
 *  0 - path is in @resolved_path
 *  1 - path is in @path (no resolution necessary)
 *  2 - errno issues -- ignore this path
 */
int resolve_dirfd_path(int dirfd, const char *path, char *resolved_path,
                       size_t resolved_path_len)
{
	/* The *at style functions have the following semantics:
	 *	- dirfd = AT_FDCWD: same as non-at func: file is based on CWD
	 *	- file is absolute: dirfd is ignored
	 *	- otherwise: file is relative to dirfd
	 * Since maintaining fd state based on open's is real messy, we'll
	 * just rely on the kernel doing it for us with /proc/<pid>/fd/ ...
	 */
	if (dirfd == AT_FDCWD || (path && path[0] == '/'))
		return 1;

	save_errno();

	size_t at_len = resolved_path_len - 1 - 1 - (path ? strlen(path) : 0);
	sprintf(resolved_path, "/proc/%i/fd/%i", trace_pid ? : getpid(), dirfd);
	ssize_t ret = readlink(resolved_path, resolved_path, at_len);
	if (ret == -1) {
		/* see comments at end of check_syscall() */
		if (errno_is_too_long()) {
			restore_errno();
			return 2;
		}
		sb_debug_dyn("AT_FD LOOKUP fail: %s: %s\n", resolved_path, strerror(errno));
		/* If the fd isn't found, some guys (glibc) expect errno */
		if (errno == ENOENT)
			errno = EBADF;
		return -1;
	}
	resolved_path[ret] = '/';
	resolved_path[ret + 1] = '\0';
	if (path)
		strcat(resolved_path, path);

	restore_errno();
	return 0;
}

int canonicalize(const char *path, char *resolved_path)
{
	int old_errno = errno;
	char *retval;

	*resolved_path = '\0';

	/* If path == NULL, return or we get a segfault */
	if (NULL == path) {
		errno = EINVAL;
		return -1;
	}

	/* Do not try to resolve an empty path */
	if ('\0' == path[0]) {
		errno = old_errno;
		return 0;
	}

	/* We can't handle resolving a buffer inline (erealpath),
	 * so demand separate read and write strings.
	 */
	sb_assert(path != resolved_path);

	retval = erealpath(path, resolved_path);

	if ((NULL == retval) && (path[0] != '/')) {
		/* The path could not be canonicalized, append it
		 * to the current working directory if it was not
		 * an absolute path
		 */

		if (errno_is_too_long())
			return -1;

		if (NULL == egetcwd(resolved_path, SB_PATH_MAX - 2))
			return -1;
		size_t len = strlen(resolved_path);
		snprintf(resolved_path + len, SB_PATH_MAX - len, "/%s", path);

		char *copy = xstrdup(resolved_path);
		char *ret = erealpath(copy, resolved_path);
		free(copy);
		if (ret == NULL) {
			if (errno_is_too_long()) {
				/* The resolved path is too long for the buffer to hold */
				return -1;
			} else {
				/* Whatever it resolved, is not a valid path */
				errno = ENOENT;
				return -1;
			}
		}

	} else if ((NULL == retval) && (path[0] == '/')) {
		/* Whatever it resolved, is not a valid path */
		errno = ENOENT;
		return -1;
	}

	errno = old_errno;
	return 0;
}

static char *resolve_path(const char *path, int follow_link)
{
	char *dname, *bname;
	char *filtered_path;

	if (NULL == path)
		return NULL;

	save_errno();

	filtered_path = xmalloc(SB_PATH_MAX * sizeof(char));

	if (0 == follow_link) {
		if (-1 == canonicalize(path, filtered_path)) {
			free(filtered_path);
			filtered_path = NULL;
		}
	} else {
		/* Basically we get the realpath which should resolve symlinks,
		 * etc.  If that fails (might not exist), we try to get the
		 * realpath of the parent directory, as that should hopefully
		 * exist.  If all else fails, just go with canonicalize */
		char *ret;
		if (trace_pid)
			ret = erealpath(path, filtered_path);
		else
			ret = realpath(path, filtered_path);

		/* Handle broken symlinks.  This can come up for a variety of reasons,
		 * but we need to make sure that we resolve the path all the way to the
		 * final target, and not just where the current link happens to start.
		 * Latest discussion is in #540828.
		 *
		 * Maybe we failed because of funky anonymous fd symlinks.
		 * You can see this by doing something like:
		 *		$ echo | ls -l /proc/self/fd/
		 *		.......	0 -> pipe:[9422999]
		 * So any syntax like this we should allow as there isn't any
		 * actual file paths for us to check against. #288863
		 * Don't look for any particular string as these are dynamic
		 * according to the kernel.  You can see pipe:, socket:, etc...
		 *
		 * Maybe we failed because it's a symlink to a path in /proc/ that
		 * is a symlink to a path that longer exists -- readlink will set
		 * ENOENT even in that case and the file ends in (deleted).  This
		 * can come up in cases like:
		 * /dev/stderr -> fd/2 -> /proc/self/fd/2 -> /removed/file (deleted)
		 */
		if (!ret && errno == ENOENT) {
			ret = canonicalize_filename_mode(path, CAN_ALL_BUT_LAST);
			if (ret) {
				free(filtered_path);
				filtered_path = ret;
			}
		}

		if (!ret) {
			char tmp_str1[SB_PATH_MAX];
			snprintf(tmp_str1, SB_PATH_MAX, "%s", path);

			dname = dirname(tmp_str1);

			/* If not, then check if we can resolve the
			 * parent directory */
			if (trace_pid)
				ret = erealpath(dname, filtered_path);
			else
				ret = realpath(dname, filtered_path);
			if (!ret) {
				/* Fall back to canonicalize */
				if (-1 == canonicalize(path, filtered_path)) {
					free(filtered_path);
					filtered_path = NULL;
				}
			} else {
				char tmp_str2[SB_PATH_MAX];
				/* OK, now add the basename to keep our access
				 * checking happy (don't want '/usr/lib' if we
				 * tried to do something with non-existing
				 * file '/usr/lib/cf*' ...) */
				snprintf(tmp_str2, SB_PATH_MAX, "%s", path);

				bname = basename(tmp_str2);
				size_t len = strlen(filtered_path);
				snprintf(filtered_path + len, SB_PATH_MAX - len, "%s%s",
					(filtered_path[len - 1] != '/') ? "/" : "",
					bname);
			}
		}
	}

	/* If things failed, don't restore errno.  More info at comment at
	 * end of check_syscall() function.
	 */
	if (filtered_path)
		restore_errno();

	return filtered_path;
}

/*
 * Internal Functions
 */

char *egetcwd(char *buf, size_t size)
{
	struct stat st;
	char *tmpbuf;

	/* We can't let the C lib allocate memory for us since we have our
	 * own local routines to handle things.
	 */
	bool allocated = (buf == NULL);
	if (allocated) {
		size = SB_PATH_MAX;
		buf = xmalloc(size);
	}

	/* If tracing a child, our cwd may not be the same as the child's */
	if (trace_pid) {
		char proc[20];
		sprintf(proc, "/proc/%i/cwd", trace_pid);
		ssize_t ret = readlink(proc, buf, size);
		if (ret == -1) {
			errno = ESRCH;
			return NULL;
		}
		buf[ret] = '\0';
		return buf;
	}

	/* Need to disable sandbox, as on non-linux libc's, opendir() is
	 * used by some getcwd() implementations and resolves to the sandbox
	 * opendir() wrapper, causing infinit recursion and finially crashes.
	 */
	sandbox_on = false;
	errno = 0;
	tmpbuf = sb_unwrapped_getcwd(buf, size);
	sandbox_on = true;

	/* We basically try to figure out if we can trust what getcwd()
	 * returned.  If one of the following happens kernel/libc side,
	 * bad things will happen, but not much we can do about it:
	 *  - Invalid pointer with errno = 0
	 *  - Truncated path with errno = 0
	 *  - Whatever I forgot about
	 */
	if ((tmpbuf) && (errno == 0)) {
		save_errno();
		if (!lstat(buf, &st))
			/* errno is set only on failure */
			errno = 0;

		if (errno == ENOENT)
			/* If lstat() failed with eerror = ENOENT, then its
			 * possible that we are running on an older kernel
			 * which had issues with returning invalid paths if
			 * they got too long.  Return with errno = ENAMETOOLONG,
			 * so that canonicalize() and check_syscall() know
			 * what the issue is.
			 */
		  	errno = ENAMETOOLONG;

		if (errno && errno != EACCES) {
			/* If getcwd() allocated the buffer, free it. */
			if (allocated)
				free(buf);

			/* Not sure if we should quit here, but I guess if
			 * lstat() fails, getcwd could have messed up. Not
			 * sure what to do about errno - use lstat()'s for
			 * now.
			 */
			return NULL;
		}

		restore_errno();
	} else if (errno != 0) {
		/* If getcwd() allocated the buffer, free it. */
		if (allocated)
			free(buf);

		/* Make sure we do not return garbage if the current libc or
		 * kernel's getcwd() is buggy.
		 */
		return NULL;
	}

	return tmpbuf;
}

void __sb_dump_backtrace(void)
{
	const char *cmdline = sb_get_cmdline(trace_pid);
	sb_printf("%s: ", cmdline);
	sb_copy_file_to_fd(cmdline, STDERR_FILENO);
	sb_printf("\n\n");
}

#define _SB_WRITE_STR(str) \
	do { \
		size_t _len = strlen(str); \
		if (sb_write(logfd, str, _len) != _len) \
			goto error; \
	} while (0)
static bool write_logfile(const char *logfile, const char *func, const char *path,
                          const char *apath, const char *rpath, bool access)
{
	struct stat log_stat;
	int stat_ret;
	int logfd;
	bool ret = false;

	stat_ret = lstat(logfile, &log_stat);
	/* Do not care about failure */
	errno = 0;
	if (stat_ret == 0 && S_ISREG(log_stat.st_mode) == 0)
		sb_ebort("SECURITY BREACH: '%s' %s\n", logfile,
			"already exists and is not a regular file!");

	logfd = sb_open(logfile,
		O_APPEND | O_WRONLY | O_CREAT | O_CLOEXEC,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (logfd == -1) {
		sb_eerror("ISE:%s: unable to append logfile: %s\n",
			__func__, logfile);
		goto error;
	}

	if (0 != stat_ret)
		_SB_WRITE_STR(
			LOG_STRING
			LOG_FMT_FUNC
			LOG_FMT_ACCESS
			LOG_FMT_PATH
			LOG_FMT_APATH
			LOG_FMT_RPATH
			LOG_FMT_CMDLINE
		);
	/* Already have data in the log, so add a newline to space the
	 * log entries.
	 */

	_SB_WRITE_STR("\nF: ");
	_SB_WRITE_STR(func);
	_SB_WRITE_STR("\nS: ");
	if (access)
		_SB_WRITE_STR("allow");
	else
		_SB_WRITE_STR("deny");
	_SB_WRITE_STR("\nP: ");
	_SB_WRITE_STR(path);
	_SB_WRITE_STR("\nA: ");
	_SB_WRITE_STR(apath);
	_SB_WRITE_STR("\nR: ");
	_SB_WRITE_STR(rpath);

	_SB_WRITE_STR("\nC: ");
	const char *cmdline = sb_get_cmdline(trace_pid);
	if (sb_copy_file_to_fd(cmdline, logfd)) {
		_SB_WRITE_STR("unable to read ");
		_SB_WRITE_STR(cmdline);
	}
	_SB_WRITE_STR("\n");

	ret = true;

 error:
	sb_close(logfd);

	return ret;
}

static void clean_env_entries(char ***prefixes_array, int *prefixes_num)
{
	if (*prefixes_array == NULL)
		return;

	size_t i;
	save_errno();

	for (i = 0; i < *prefixes_num; ++i) {
		if (NULL != (*prefixes_array)[i]) {
			free((*prefixes_array)[i]);
			(*prefixes_array)[i] = NULL;
		}
	}
	if (NULL != *prefixes_array)
		free(*prefixes_array);
	*prefixes_array = NULL;
	*prefixes_num = 0;

	restore_errno();
}

#define pfx_num		(*prefixes_num)
#define pfx_array	(*prefixes_array)
#define pfx_item	((*prefixes_array)[(*prefixes_num)])

static void init_env_entries(char ***prefixes_array, int *prefixes_num, const char *env, const char *prefixes_env, int warn)
{
	char *token = NULL;
	char *rpath = NULL;
	char *buffer = NULL;
	char *buffer_ptr = NULL;
	int prefixes_env_length = strlen(prefixes_env);
	int num_delimiters = 0;
	int i = 0;
	int old_errno = errno;

	if (NULL == prefixes_env) {
		/* Do not warn if this is in init stage, as we might get
		 * issues due to LD_PRELOAD already set (bug #91431). */
		if (sb_init)
			fprintf(stderr,
				"libsandbox:  The '%s' env variable is not defined!\n",
				env);
		if (pfx_array) {
			for (i = 0; i < pfx_num; i++)
				free(pfx_item);
			free(pfx_array);
		}
		pfx_num = 0;

		goto done;
	}

	for (i = 0; i < prefixes_env_length; i++) {
		if (':' == prefixes_env[i])
			num_delimiters++;
	}

	/* num_delimiters might be 0, and we need 2 entries at least */
	pfx_array = xmalloc(((num_delimiters * 2) + 2) * sizeof(char *));
	buffer = xstrdup(prefixes_env);
	buffer_ptr = buffer;

#ifdef HAVE_STRTOK_R
	token = strtok_r(buffer_ptr, ":", &buffer_ptr);
#else
	token = strtok(buffer_ptr, ":");
#endif

	while ((NULL != token) && (strlen(token) > 0)) {
		pfx_item = resolve_path(token, 0);
		/* We do not care about errno here */
		errno = 0;
		if (NULL != pfx_item) {
			pfx_num++;

			/* Now add the realpath if it exists and
			 * are not a duplicate */
			rpath = xmalloc(SB_PATH_MAX * sizeof(char));
			pfx_item = realpath(*(&(pfx_item) - 1), rpath);
			if ((NULL != pfx_item) &&
			    (0 != strcmp(*(&(pfx_item) - 1), pfx_item))) {
				pfx_num++;
			} else {
				free(rpath);
				pfx_item = NULL;
			}
		}

#ifdef HAVE_STRTOK_R
		token = strtok_r(NULL, ":", &buffer_ptr);
#else
		token = strtok(NULL, ":");
#endif
	}

	free(buffer);

done:
	errno = old_errno;
	return;
}

static void sb_process_env_settings(void)
{
	static const char * const sb_env_names[4] = {
		ENV_SANDBOX_DENY,
		ENV_SANDBOX_READ,
		ENV_SANDBOX_WRITE,
		ENV_SANDBOX_PREDICT,
	};

	size_t i;
	for (i = 0; i < ARRAY_SIZE(sb_env_names); ++i) {
		char *sb_env = getenv(sb_env_names[i]);

		/* Allow the vars to change values, but not be unset.
		 * See sb_check_envp() for more details. */
		if (!sb_env)
			continue;

		if (/*(!sb_env && cached_env_vars[i]) || -- see above */
		    !cached_env_vars[i] ||
		    strcmp(cached_env_vars[i], sb_env) != 0)
		{
			clean_env_entries(&sbcontext.prefixes[i], &sbcontext.num_prefixes[i]);

			if (cached_env_vars[i])
				free(cached_env_vars[i]);

			if (sb_env) {
				init_env_entries(&sbcontext.prefixes[i], &sbcontext.num_prefixes[i],
					sb_env_names[i], sb_env, 1);
				cached_env_vars[i] = xstrdup(sb_env);
			} else
				cached_env_vars[i] = NULL;
		}
	}
}

static int check_prefixes(char **prefixes, int num_prefixes, const char *path)
{
	if (!prefixes)
		return 0;

	size_t i;
	for (i = 0; i < num_prefixes; ++i) {
		if (unlikely(!prefixes[i]))
			continue;

		size_t prefix_len = strlen(prefixes[i]);
		/* Start with a regular prefix match for speed */
		if (strncmp(path, prefixes[i], prefix_len))
			continue;

		/* Now, if prefix did not end with a slash, we need to make sure
		 * we are not matching in the middle of a filename. So check
		 * whether the match is followed by a slash, or NUL.
		 */
		if (prefixes[i][prefix_len-1] != '/'
				&& path[prefix_len] != '/' && path[prefix_len] != '\0')
			continue;

		return 1;
	}

	return 0;
}

/* Is this a func that works on symlinks, and is the file a symlink ? */
static bool symlink_func(int sb_nr, int flags, const char *abs_path)
{
	struct stat st;

	/* These funcs always operate on symlinks */
	if (!(sb_nr == SB_NR_UNLINK   ||
	      sb_nr == SB_NR_UNLINKAT ||
	      sb_nr == SB_NR_LCHOWN   ||
	      sb_nr == SB_NR_REMOVE   ||
	      sb_nr == SB_NR_RENAME   ||
	      sb_nr == SB_NR_RENAMEAT ||
	      sb_nr == SB_NR_RMDIR    ||
	      sb_nr == SB_NR_SYMLINK  ||
	      sb_nr == SB_NR_SYMLINKAT))
	{
		/* These funcs sometimes operate on symlinks */
		if (!((sb_nr == SB_NR_FCHOWNAT ||
		       sb_nr == SB_NR_FCHMODAT ||
		       sb_nr == SB_NR_UTIMENSAT) &&
		      (flags & AT_SYMLINK_NOFOLLOW)))
			return false;
	}

	if (-1 != lstat(abs_path, &st) && S_ISLNK(st.st_mode))
		return true;
	else
		return false;
}

static int check_access(sbcontext_t *sbcontext, int sb_nr, const char *func,
                        int flags, const char *abs_path, const char *resolv_path)
{
	int old_errno = errno;
	int result = 0;
	int retval;
	bool sym_func = symlink_func(sb_nr, flags, abs_path);

	retval = check_prefixes(sbcontext->deny_prefixes,
		sbcontext->num_deny_prefixes, abs_path);
	if (1 == retval)
		/* Fall in a read/write denied path, Deny Access */
		goto out;

	if (!sym_func) {
		retval = check_prefixes(sbcontext->deny_prefixes,
			sbcontext->num_deny_prefixes, resolv_path);
		if (1 == retval)
			/* Fall in a read/write denied path, Deny Access */
			goto out;
	}

	/* Hardcode denying write to the whole log dir.  While this is a
	 * parial match and so rejects paths that also start with this
	 * string, that isn't going to happen in real life so live with
	 * it.  We can't append a slash to this path either as that would
	 * allow people to open the dir itself for writing.
	 */
	if (!strncmp(resolv_path, SANDBOX_LOG_LOCATION, strlen(SANDBOX_LOG_LOCATION)))
		goto out;

	if (sbcontext->read_prefixes &&
	    (sb_nr == SB_NR_ACCESS_RD ||
	     sb_nr == SB_NR_OPEN_RD   ||
	     sb_nr == SB_NR_OPENDIR   ||
	     sb_nr == SB_NR_POPEN     ||
	     sb_nr == SB_NR_SYSTEM    ||
	   /*sb_nr == SB_NR_EXECL     ||
	     sb_nr == SB_NR_EXECLP    ||
	     sb_nr == SB_NR_EXECLE    ||*/
	     sb_nr == SB_NR_EXECV     ||
	     sb_nr == SB_NR_EXECVP    ||
	     sb_nr == SB_NR_EXECVE    ||
	     sb_nr == SB_NR_EXECVPE   ||
	     sb_nr == SB_NR_FEXECVE))
	{
		retval = check_prefixes(sbcontext->read_prefixes,
					sbcontext->num_read_prefixes, resolv_path);
		if (1 == retval) {
			/* Fall in a readable path, Grant Access */
			result = 1;
			goto out;
		}

		/* If we are here, and still no joy, and its the access() call,
		 * do not log it, but just return -1 */
		if (sb_nr == SB_NR_ACCESS_RD) {
			sbcontext->show_access_violation = false;
			goto out;
		}
	}

	if (sb_nr == SB_NR_ACCESS_WR   ||
	    sb_nr == SB_NR_CHMOD       ||
	    sb_nr == SB_NR_CHOWN       ||
	    sb_nr == SB_NR_CREAT       ||
	    sb_nr == SB_NR_CREAT64     ||
	    sb_nr == SB_NR_FCHMODAT    ||
	    sb_nr == SB_NR_FCHOWNAT    ||
	  /*sb_nr == SB_NR_FTRUNCATE   ||
	    sb_nr == SB_NR_FTRUNCATE64 ||*/
	    sb_nr == SB_NR_FUTIMESAT   ||
	    sb_nr == SB_NR_LCHOWN      ||
	    sb_nr == SB_NR_LINK        ||
	    sb_nr == SB_NR_LINKAT      ||
	    sb_nr == SB_NR_LUTIMES     ||
	    sb_nr == SB_NR_MKDIR       ||
	    sb_nr == SB_NR_MKDIRAT     ||
	    sb_nr == SB_NR_MKDTEMP     ||
	    sb_nr == SB_NR_MKFIFO      ||
	    sb_nr == SB_NR_MKFIFOAT    ||
	    sb_nr == SB_NR_MKNOD       ||
	    sb_nr == SB_NR_MKNODAT     ||
	    sb_nr == SB_NR_MKOSTEMP    ||
	    sb_nr == SB_NR_MKOSTEMP64  ||
	    sb_nr == SB_NR_MKOSTEMPS   ||
	    sb_nr == SB_NR_MKOSTEMPS64 ||
	    sb_nr == SB_NR_MKSTEMP     ||
	    sb_nr == SB_NR_MKSTEMP64   ||
	    sb_nr == SB_NR_MKSTEMPS    ||
	    sb_nr == SB_NR_MKSTEMPS64  ||
	    sb_nr == SB_NR_OPEN_WR     ||
	    sb_nr == SB_NR_REMOVE      ||
	    sb_nr == SB_NR_RENAME      ||
	    sb_nr == SB_NR_RENAMEAT    ||
	    sb_nr == SB_NR_RMDIR       ||
	    sb_nr == SB_NR_SYMLINK     ||
	    sb_nr == SB_NR_SYMLINKAT   ||
	    sb_nr == SB_NR_TRUNCATE    ||
	    sb_nr == SB_NR_TRUNCATE64  ||
	    sb_nr == SB_NR_UNLINK      ||
	    sb_nr == SB_NR_UNLINKAT    ||
	    sb_nr == SB_NR_UTIME       ||
	    sb_nr == SB_NR_UTIMENSAT   ||
	    sb_nr == SB_NR_UTIMES      ||
	    sb_nr == SB_NR__XMKNOD     ||
	    sb_nr == SB_NR___XMKNOD    ||
	    sb_nr == SB_NR___XMKNODAT)
	{

		retval = check_prefixes(sbcontext->write_denied_prefixes,
					sbcontext->num_write_denied_prefixes,
					resolv_path);
		if (1 == retval)
			/* Falls in a write denied path, Deny Access */
			goto out;

		retval = check_prefixes(sbcontext->write_prefixes,
					sbcontext->num_write_prefixes, resolv_path);
		if (1 == retval) {
			/* Falls in a writable path, Grant Access */
			result = 1;
			goto out;
		}

		/* Hack to allow writing to '/proc/self/fd' #91516.  It needs
		 * to be here as for each process, the '/proc/self' symlink
		 * will differ ...
		 */
		char proc_self_fd[SB_PATH_MAX];
		if (realpath(sb_get_fd_dir(), proc_self_fd) &&
		    !strncmp(resolv_path, proc_self_fd, strlen(proc_self_fd)))
		{
			result = 1;
			goto out;
		}

		/* If operating on a location those parent dirs do not exist,
		 * then let it through as the OS itself will trigger a fail.
		 * This is like fopen("/foo/bar", "w") and /foo/ does not
		 * exist.  All the functions filtered thus far fall into that
		 * behavior category, so no need to check the syscall.
		 */
		char *dname_buf = xstrdup(resolv_path);
		int aret = sb_unwrapped_access(dirname(dname_buf), F_OK);
		free(dname_buf);
		if (aret) {
			result = 1;
			goto out;
		}

		retval = check_prefixes(sbcontext->predict_prefixes,
					sbcontext->num_predict_prefixes, resolv_path);
		if (1 == retval) {
			/* Is a known access violation, so deny access,
			 * and do not log it */
			sbcontext->show_access_violation = false;
			goto out;
		}

		/* A very common bug (apparently) is for .py[co] files to fall out
		 * of sync with their .py source files.  Rather than trigger a hard
		 * failure, let's just whine about it.  Once python itself gets
		 * sorted out, we can drop this #256953.
		 */
		size_t len = strlen(resolv_path);
		if (len > 4) {
			const char *py = resolv_path + len - 4;
			if (!strcmp(py, ".pyc") || !strcmp(py, ".pyo")) {
				sbcontext->show_access_violation = false;
				goto out;
			}
		}

		/* If we are here, and still no joy, and its the access() call,
		 * do not log it, but just return -1 */
		if (sb_nr == SB_NR_ACCESS_WR) {
			sbcontext->show_access_violation = false;
			goto out;
		}
	}

out:
	errno = old_errno;

	return result;
}

/* Return values:
 *  0: failure, caller should abort
 *  1: things worked out fine
 *  2: things worked out fine, but the errno should not be restored
 */
static int check_syscall(sbcontext_t *sbcontext, int sb_nr, const char *func,
                         const char *file, int flags)
{
	char *absolute_path = NULL;
	char *resolved_path = NULL;
	int old_errno = errno;
	int result;
	bool access, debug, verbose, set;

	absolute_path = resolve_path(file, 0);
	/* Do not bother dereferencing symlinks when we are using a function that
	 * itself does not dereference.  This speeds things up and avoids updating
	 * the atime implicitly. #415475
	 */
	if (symlink_func(sb_nr, flags, absolute_path))
		resolved_path = absolute_path;
	else
		resolved_path = resolve_path(file, 1);
	if (!absolute_path || !resolved_path)
		goto error;
	sb_debug_dyn("absolute_path: %s\n", absolute_path);
	sb_debug_dyn("resolved_path: %s\n", resolved_path);

	verbose = is_env_set_on(ENV_SANDBOX_VERBOSE, &set);
	if (set)
		sbcontext->verbose = verbose;
	debug = is_env_set_on(ENV_SANDBOX_DEBUG, &set);
	if (set)
		sbcontext->debug = debug;

	result = check_access(sbcontext, sb_nr, func, flags, absolute_path, resolved_path);

	if (unlikely(verbose)) {
		int sym_len = SB_MAX_STRING_LEN + 1 - strlen(func);
		if (!result && sbcontext->show_access_violation)
			sb_eerror("%sACCESS DENIED%s:  %s:%*s%s\n",
				COLOR_RED, COLOR_NORMAL, func, sym_len, "", absolute_path);
		else if (debug && sbcontext->show_access_violation)
			sb_einfo("%sACCESS ALLOWED%s:  %s:%*s%s\n",
				COLOR_GREEN, COLOR_NORMAL, func, sym_len, "", absolute_path);
		else if (debug && !sbcontext->show_access_violation)
			sb_ewarn("%sACCESS PREDICTED%s:  %s:%*s%s\n",
				COLOR_YELLOW, COLOR_NORMAL, func, sym_len, "", absolute_path);
	}

	if ((0 == result) && sbcontext->show_access_violation)
		access = false;
	else
		access = true;

	if (unlikely(!access)) {
		bool worked = write_logfile(log_path, func, file, absolute_path, resolved_path, access);
		if (!worked && errno)
			goto error;
	}

	if (unlikely(debug)) {
		bool worked = write_logfile(debug_log_path, func, file, absolute_path, resolved_path, access);
		if (!worked && errno)
			goto error;
	}

	free(absolute_path);
	if (absolute_path != resolved_path)
		free(resolved_path);

	errno = old_errno;

	return result;

 error:
	/* The path is too long to be canonicalized, so just warn and let the
	 * function handle it (see bugs #21766 #94630 #101728 #227947)
	 */
	if (errno_is_too_long()) {
		free(absolute_path);
		if (absolute_path != resolved_path)
			free(resolved_path);
		return 2;
	}

	/* Process went away while we were tracing it ... #264478 */
	if (trace_pid && errno == ESRCH)
		return 2;

	/* If we get here, something bad happened */
	sb_ebort("ISE: %s(%s)\n\tabs_path: %s\n\tres_path: %s\n",
		func, file, absolute_path, resolved_path);
}

bool is_sandbox_on(void)
{
	bool result = false;
	save_errno();

	/* $SANDBOX_ACTIVE is an env variable that should ONLY
	 * be used internal by sandbox.c and libsanbox.c.  External
	 * sources should NEVER set it, else the sandbox is enabled
	 * in some cases when run in parallel with another sandbox,
	 * but not even in the sandbox shell.
	 */
	if (sandbox_on) {
		if (!sbcontext.active) {
			/* Once you go active, you never go back */
			char *sb_env_active = getenv(ENV_SANDBOX_ACTIVE);
			sbcontext.active = (sb_env_active && !strcmp(sb_env_active, SANDBOX_ACTIVE));
		}

		if (sbcontext.active) {
			bool on, set;
			on = is_env_set_on(ENV_SANDBOX_ON, &set);
			if (set)
				sbcontext.on = on;

			if (sbcontext.on)
				result = true;
		}
	}

	restore_errno();
	return result;
}

bool before_syscall(int dirfd, int sb_nr, const char *func, const char *file, int flags)
{
	int result;
	char at_file_buf[SB_PATH_MAX];

	/* Some funcs operate on a fd directly and so filename is NULL, but
	 * the rest should get rejected as "file/directory does not exist".
	 */
	if (file == NULL || file[0] == '\0') {
		if (file == NULL && dirfd != AT_FDCWD &&
			(sb_nr == SB_NR_UTIMENSAT || sb_nr == SB_NR_FUTIMESAT))
		{
			/* let it slide -- the func is magic and changes behavior
			 * from "file relative to dirfd" to "dirfd is actually file
			 * fd" whenever file is NULL.
			 */
		} else {
			errno = ENOENT;
			return false;
		}
	}

	switch (resolve_dirfd_path(dirfd, file, at_file_buf, sizeof(at_file_buf))) {
		case -1: return false;
		case 0: file = at_file_buf; break;
		case 2: return true;
	}

	save_errno();

	/* Need to protect the global sbcontext structure */
	sb_lock();

	if (!sb_init) {
		libsb_init();
		sb_init = true;
	}

	sb_process_env_settings();

	/* Might have been reset in check_access() */
	sbcontext.show_access_violation = true;

	result = check_syscall(&sbcontext, sb_nr, func, file, flags);

	sb_unlock();

	if (0 == result) {
		/* FIXME: Should probably audit errno, and enable some other
		 *        error to be returned (EINVAL for invalid mode for
		 *        fopen() and co, ETOOLONG, etc). */
		errno = EACCES;
	} else if (result == 1)
		restore_errno();

	return result ? true : false;
}

bool before_syscall_access(int dirfd, int sb_nr, const char *func, const char *file, int flags)
{
	const char *ext_func;
	if (flags & W_OK)
		sb_nr = SB_NR_ACCESS_WR, ext_func = "access_wr";
	else
		sb_nr = SB_NR_ACCESS_RD, ext_func = "access_rd";
	return before_syscall(dirfd, sb_nr, ext_func, file, flags);
}

bool before_syscall_open_int(int dirfd, int sb_nr, const char *func, const char *file, int flags)
{
	const char *ext_func;
	if ((flags & O_WRONLY) || (flags & O_RDWR))
		sb_nr = SB_NR_OPEN_WR, ext_func = "open_wr";
	else
		sb_nr = SB_NR_OPEN_RD, ext_func = "open_rd";
	return before_syscall(dirfd, sb_nr, ext_func, file, flags);
}

bool before_syscall_open_char(int dirfd, int sb_nr, const char *func, const char *file, const char *mode)
{
	if (NULL == mode)
		return false;

	const char *ext_func;
	if ((*mode == 'r') && ((0 == (strcmp(mode, "r"))) ||
	     /* The strspn accept args are known non-writable modifiers */
	     (strlen(mode+1) == strspn(mode+1, "xbtmce"))))
		sb_nr = SB_NR_OPEN_RD, ext_func = "fopen_rd";
	else
		sb_nr = SB_NR_OPEN_WR, ext_func = "fopen_wr";
	return before_syscall(dirfd, sb_nr, ext_func, file, 0);
}

typedef struct {
	const char *name;
	size_t len;
	char *value;
} env_pair;
#define ENV_PAIR(x, n, v) [x] = { .name = n, .len = sizeof(n) - 1, .value = v, }

#define str_list_add_item_env(_string_list, _var, _item, _error) \
	do { \
		char *str = xmalloc(strlen(_var) + strlen(_item) + 2); \
		sprintf(str, "%s=%s", _var, _item); \
		str_list_add_item(_string_list, str, _error); \
	} while (0)
/* We need to make sure we pass along sandbox env vars.  If we don't, programs
 * (like scons) will inadvertently disable us.  While we allow modification
 * (e.g. export SANDBOX_WRITE=""), we disallow clearing (e.g. unset SANDBOX_WRITE).
 * The former is clear in the user's intention, but the latter is indicative
 * of a bad program.
 *
 * XXX: Might be much nicer if we could serialize these vars behind the back of
 *      the program.  Might be hard to handle LD_PRELOAD though ...
 */
char **sb_check_envp(char **envp, size_t *mod_cnt, bool insert)
{
	char **my_env;
	char *entry;
	size_t count, i;
	env_pair vars[] = {
		/* Indices matter -- see init below */
		ENV_PAIR( 0, ENV_LD_PRELOAD, sandbox_lib),
		ENV_PAIR( 1, ENV_SANDBOX_LOG, log_path),
		ENV_PAIR( 2, ENV_SANDBOX_DEBUG_LOG, debug_log_path),
		ENV_PAIR( 3, ENV_SANDBOX_MESSAGE_PATH, message_path),
		ENV_PAIR( 4, ENV_SANDBOX_DENY, cached_env_vars[0]),
		ENV_PAIR( 5, ENV_SANDBOX_READ, cached_env_vars[1]),
		ENV_PAIR( 6, ENV_SANDBOX_WRITE, cached_env_vars[2]),
		ENV_PAIR( 7, ENV_SANDBOX_PREDICT, cached_env_vars[3]),
		ENV_PAIR( 8, ENV_SANDBOX_ON, NULL),
		ENV_PAIR( 9, ENV_SANDBOX_ACTIVE, NULL),
		ENV_PAIR(10, ENV_SANDBOX_VERBOSE, NULL),
		ENV_PAIR(11, ENV_SANDBOX_DEBUG, NULL),
		ENV_PAIR(12, "LD_LIBRARY_PATH", NULL),
		ENV_PAIR(13, ENV_SANDBOX_TESTING, NULL),
	};
	size_t num_vars = ARRAY_SIZE(vars);
	char *found_vars[num_vars];
	size_t found_var_cnt;

	/* First figure out how many vars are already in the env */
	found_var_cnt = 0;
	memset(found_vars, 0, sizeof(found_vars));

	str_list_for_each_item(envp, entry, count) {
		for (i = 0; i < num_vars; ++i) {
			if (found_vars[i])
				continue;
			if (unlikely(!is_env_var(entry, vars[i].name, vars[i].len)))
				continue;
			found_vars[i] = entry;
			++found_var_cnt;
		}
	}

	/* Now specially handle merging of LD_PRELOAD */
	char *ld_preload;
	bool merge_ld_preload = found_vars[0] && !strstr(found_vars[0], sandbox_lib);
	if (unlikely(merge_ld_preload)) {
		/* Ok, there's an existing LD_PRELOAD value that we need to merge
		 * with.  Handle this specially. */
		size_t ld_preload_len = strlen(ENV_LD_PRELOAD);
		count = ld_preload_len + 1 + strlen(sandbox_lib) + 1 +
			strlen(found_vars[0] + ld_preload_len + 1);
		ld_preload = xmalloc(count * sizeof(char));
		sprintf(ld_preload, "%s=%s %s", ENV_LD_PRELOAD, sandbox_lib,
			found_vars[0] + ld_preload_len + 1);
		goto mod_env;
	}

	/* If we found everything, there's nothing to do! */
	if ((insert && num_vars == found_var_cnt) ||
	    (!insert && found_var_cnt == 0))
		/* Use the user's envp */
		return envp;

	/* Ok, we need to create our own envp, as we need to restore stuff
	 * and we should not touch the user's envp.  First we add our vars,
	 * and just all the rest. */
 mod_env:
	/* Indices matter -- see vars[] setup above */
	if (sbcontext.on)
		vars[8].value = "1";
	if (sbcontext.active)
		vars[9].value = SANDBOX_ACTIVE;
	if (sbcontext.verbose)
		vars[10].value = "1";
	if (sbcontext.debug)
		vars[11].value = "1";
	if (sbcontext.testing) {
		vars[12].value = sbcontext.ld_library_path;
		vars[13].value = "1";
	}

	my_env = NULL;
	if (!insert) {
		if (mod_cnt) {
			str_list_for_each_item(envp, entry, count) {
				for (i = 0; i < num_vars; ++i)
					if (i != 12 && is_env_var(entry, vars[i].name, vars[i].len)) {
						(*mod_cnt)++;
						goto skip;
					}
				str_list_add_item(my_env, entry, error);
 skip: ;
			}
		} else {
			for (i = 0; i < num_vars; ++i)
				if (i != 12) unsetenv(vars[i].name);
		}
	} else {
		if (mod_cnt) {
			/* Count directly due to variability with LD_PRELOAD and the value
			 * logic below.  Getting out of sync can mean memory corruption. */
			*mod_cnt = 0;
			if (unlikely(merge_ld_preload)) {
				str_list_add_item(my_env, ld_preload, error);
				(*mod_cnt)++;
			}
			for (i = 0; i < num_vars; ++i) {
				if (found_vars[i] || !vars[i].value)
					continue;
				str_list_add_item_env(my_env, vars[i].name, vars[i].value, error);
				(*mod_cnt)++;
			}

			str_list_for_each_item(envp, entry, count) {
				if (unlikely(merge_ld_preload && is_env_var(entry, vars[0].name, vars[0].len)))
					continue;
				str_list_add_item(my_env, entry, error);
			}
		} else {
			if (unlikely(merge_ld_preload))
				putenv(ld_preload);
			for (i = 0; i < num_vars; ++i) {
				if (found_vars[i] || !vars[i].value)
					continue;
				setenv(vars[i].name, vars[i].value, 1);
			}
		}
	}

 error:
	return my_env;
}

void sb_cleanup_envp(char **envp, size_t mod_cnt)
{
	/* We assume all the stuffed vars are at the start */
	size_t i;
	for (i = 0; i < mod_cnt; ++i)
		free(envp[i]);

	/* We do not use str_list_free(), as we did not allocate the
	 * entries except for LD_PRELOAD.  All the other entries are
	 * pointers to existing envp memory.
	 */
	free(envp);
}
