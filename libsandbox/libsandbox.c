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
#include <alloca.h>

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
	sandbox_method_t method;
	char *ld_library_path;
	char *prefixes[5];
#define             deny_prefixes     prefixes[0]
#define             read_prefixes     prefixes[1]
#define            write_prefixes     prefixes[2]
#define          predict_prefixes     prefixes[3]
#define     write_denied_prefixes     prefixes[4]
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
int (*sbio_faccessat)(int, const char *, int, int) = sb_unwrapped_faccessat;
int (*sbio_open)(const char *, int, mode_t) = sb_unwrapped_open;
FILE *(*sbio_popen)(const char *, const char *) = sb_unwrapped_popen;

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
	sbcontext.method = get_sandbox_method();
	if (sbcontext.testing) {
		const char *ldpath = getenv("LD_LIBRARY_PATH");
		if (ldpath)
			sbcontext.ld_library_path = xstrdup(ldpath);
	}
}

sandbox_method_t get_sandbox_method(void)
{
	return parse_sandbox_method(getenv(ENV_SANDBOX_METHOD));
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

static size_t strv_append(char **basep, size_t offset, const char *value)
{
	char *base = *basep;
	size_t bsize = malloc_size(base);
	size_t vsize = strlen(value) + 1;

	if (offset + vsize >= bsize)
		*basep = base = xrealloc(base, offset + vsize + 1);

	memcpy(base + offset, value, vsize);
	offset += vsize;
	base[offset] = '\0';

	return offset;
}

static void init_env_entries(char **prefixes, const char *env, const char *prefixes_env)
{
	char *token = NULL;
	char *buffer = NULL;
	char *buffer_ptr = NULL;
	int old_errno = errno;

	if (NULL == prefixes_env) {
		/* Do not warn if this is in init stage, as we might get
		 * issues due to LD_PRELOAD already set (bug #91431). */
		if (sb_init)
			fprintf(stderr,
				"libsandbox:  The '%s' env variable is not defined!\n",
				env);

		goto done;
	}

	buffer = xstrdup(prefixes_env);
	buffer_ptr = buffer;

#ifdef HAVE_STRTOK_R
	token = strtok_r(buffer_ptr, ":", &buffer_ptr);
#else
	token = strtok(buffer_ptr, ":");
#endif

	size_t offset = 0;

	while (token && strlen(token) > 0) {
		char buf[SB_PATH_MAX];
		if (sb_abspathat(AT_FDCWD, token, buf, sizeof(buf))) {
			size_t prev_offset = offset;
			offset = strv_append(prefixes, offset, buf);
			/* Now add the realpath if it exists and
			 * are not a duplicate */
			if (sb_realpathat(AT_FDCWD, token, buf, sizeof(buf), 0, false))
				if (strcmp(buf, *prefixes + prev_offset))
					offset = strv_append(prefixes, offset, buf);
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
			free(sbcontext.prefixes[i]);
			sbcontext.prefixes[i] = NULL;
			free(cached_env_vars[i]);

			if (sb_env) {
				init_env_entries(sbcontext.prefixes + i, sb_env_names[i], sb_env);
				cached_env_vars[i] = xstrdup(sb_env);
			} else
				cached_env_vars[i] = NULL;
		}
	}
}

static bool check_prefixes(char *prefixes, const char *path)
{
	if (!prefixes)
		return false;

	size_t prefix_len;

	for (const char *prefix = prefixes; *prefix; prefix += prefix_len + 1) {
		prefix_len = strlen(prefix);

		/* Start with a regular prefix match for speed */
		if (strncmp(path, prefix, prefix_len))
			continue;

		/* Now, if prefix did not end with a slash, we need to make sure
		 * we are not matching in the middle of a filename. So check
		 * whether the match is followed by a slash, or NUL.
		 */
		if (prefix[prefix_len - 1] != '/' && path[prefix_len] != '/' && path[prefix_len] != '\0')
			continue;

		return true;
	}

	return false;
}

static int check_access(sbcontext_t *sbcontext, int sb_nr, const char *func,
                        int flags, const char *abs_path, const char *resolv_path)
{
	int old_errno = errno;
	int result = 0;

	/* Do not check non-filesystem objects like pipes and sockets */
	/* Allow operations on memfd objects #910561 */
	if (resolv_path[0] != '/' || !strncmp(resolv_path, "/memfd:", 7)) {
		result = 1;
		goto out;
	}

	if (check_prefixes(sbcontext->deny_prefixes, abs_path))
		/* Fall in a read/write denied path, Deny Access */
		goto out;

	if (check_prefixes(sbcontext->deny_prefixes, resolv_path))
		/* Fall in a read/write denied path, Deny Access */
		goto out;

	if (sbcontext->read_prefixes &&
	    (sb_nr == SB_NR_ACCESS_RD ||
	     sb_nr == SB_NR_OPEN_RD   ||
	     sb_nr == SB_NR_OPEN_RD_CREAT ||
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
		if (check_prefixes(sbcontext->read_prefixes, resolv_path)) {
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

	/* Hardcode denying write to the whole log dir.  While this is a
	 * parial match and so rejects paths that also start with this
	 * string, that isn't going to happen in real life so live with
	 * it.  We can't append a slash to this path either as that would
	 * allow people to open the dir itself for writing.
	 */
	if (!strncmp(resolv_path, SANDBOX_LOG_LOCATION, strlen(SANDBOX_LOG_LOCATION)))
		goto out;

	if (sb_nr == SB_NR_ACCESS_WR   ||
	    sb_nr == SB_NR_CHMOD       ||
	    sb_nr == SB_NR_CHOWN       ||
	    sb_nr == SB_NR_CREAT       ||
	    sb_nr == SB_NR_CREAT64     ||
	    sb_nr == SB_NR_FCHMOD      ||
	    sb_nr == SB_NR_FCHMODAT    ||
	    sb_nr == SB_NR_FCHOWN      ||
	    sb_nr == SB_NR_FCHOWNAT    ||
	  /*sb_nr == SB_NR_FTRUNCATE   ||
	    sb_nr == SB_NR_FTRUNCATE64 ||*/
	    sb_nr == SB_NR_FUTIMESAT   ||
	    sb_nr == SB_NR_LCHOWN      ||
	    sb_nr == SB_NR_LINK        ||
	    sb_nr == SB_NR_LINKAT      ||
	    sb_nr == SB_NR_LREMOVEXATTR||
	    sb_nr == SB_NR_LSETXATTR   ||
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
	    sb_nr == SB_NR_OPEN_WR_CREAT ||
	    sb_nr == SB_NR_REMOVE      ||
	    sb_nr == SB_NR_REMOVEXATTR ||
	    sb_nr == SB_NR_RENAME      ||
	    sb_nr == SB_NR_RENAMEAT    ||
	    sb_nr == SB_NR_RENAMEAT2   ||
	    sb_nr == SB_NR_RMDIR       ||
	    sb_nr == SB_NR_SETXATTR    ||
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

		if (check_prefixes(sbcontext->write_denied_prefixes, resolv_path))
			/* Falls in a write denied path, Deny Access */
			goto out;

		if (check_prefixes(sbcontext->write_prefixes, resolv_path)) {
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

		if (check_prefixes(sbcontext->predict_prefixes, resolv_path)) {
			/* Is a known access violation, so deny access,
			 * and do not log it */
			sbcontext->show_access_violation = false;
			goto out;
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

static bool is_symlink_func(int sb_nr)
{
	switch (sb_nr) {
		case SB_NR_UNLINK:
		case SB_NR_UNLINKAT:
		case SB_NR_LCHOWN:
		case SB_NR_LREMOVEXATTR:
		case SB_NR_LSETXATTR:
		case SB_NR_LUTIMES:
		case SB_NR_REMOVE:
		case SB_NR_RENAME:
		case SB_NR_RENAMEAT:
		case SB_NR_RENAMEAT2:
		case SB_NR_RMDIR:
		case SB_NR_SYMLINK:
		case SB_NR_SYMLINKAT:
			return true;
		default:
			return false;
	}
}

static bool is_create(int sb_nr)
{
	switch (sb_nr) {
		case SB_NR_CREAT:
		case SB_NR_CREAT64:
		case SB_NR_LINK:
		case SB_NR_LINKAT:
		case SB_NR_MKDIR:
		case SB_NR_MKDIRAT:
		case SB_NR_MKDTEMP:
		case SB_NR_MKFIFO:
		case SB_NR_MKNOD:
		case SB_NR_MKNODAT:
		case SB_NR_MKSTEMP:
		case SB_NR_MKSTEMP64:
		case SB_NR_MKSTEMPS:
		case SB_NR_MKSTEMPS64:
		case SB_NR_MKOSTEMP:
		case SB_NR_MKOSTEMP64:
		case SB_NR_MKOSTEMPS:
		case SB_NR_MKOSTEMPS64:
		case SB_NR_OPEN_RD_CREAT:
		case SB_NR_OPEN_WR_CREAT:
		case SB_NR_RENAME:
		case SB_NR_RENAMEAT:
		case SB_NR_RENAMEAT2:
		case SB_NR_SYMLINK:
		case SB_NR_SYMLINKAT:
		case SB_NR__XMKNOD:
		case SB_NR___XMKNOD:
		case SB_NR___XMKNODAT:
			return true;
		default:
			return false;
	}
}

static int get_pid_fd(pid_t pid, int dirfd)
{
	char path[33];
	int r;

	if (dirfd == AT_FDCWD)
		sprintf(path, "/proc/%i/cwd", pid);
	else
		sprintf(path, "/proc/%i/fd/%i", pid, dirfd);

	r = sb_unwrapped_open(path, O_PATH|O_CLOEXEC, 0);

	if (r < 0 && errno == ENOENT)
		errno = EBADF;

	return r;
}

static void cleanup_free(void *vp)
{
	void **vpp = vp;
	free(*vpp);
}

#define _cleanup_free_ __attribute__((cleanup(cleanup_free)))

/* Return values:
 *  0: failure, caller should abort
 *  1: things worked out fine
 *  2: things worked out fine, but the errno should not be restored
 */
static int check_syscall(sbcontext_t *sbcontext, int sb_nr, const char *func,
                         int dirfd, const char *file, int flags)
{
	int old_errno = errno;
	int result;
	bool access, debug, verbose, set;
	char *absolute_path, *resolved_path;
	_cleanup_free_ char *abuf = NULL;
	_cleanup_free_ char *rbuf = NULL;

	int trace_dirfd = -1;
	if (trace_pid && (file == NULL || file[0] != '/')) {
		trace_dirfd = get_pid_fd(trace_pid, dirfd);
		if (trace_dirfd < 0)
			return 2;
		dirfd = trace_dirfd;
	}

	if (sb_nr == SB_NR_OPEN_WR || sb_nr == SB_NR_OPEN_WR_CREAT) {
		struct stat st;
		if (!fstatat(dirfd, file, &st, flags) && S_ISDIR(st.st_mode))
			return 1; /* let the kernel return EISDIR */
	}

	if (is_symlink_func(sb_nr))
		flags |= AT_SYMLINK_NOFOLLOW;

	absolute_path = abuf = malloc(SB_PATH_MAX);
	if (!absolute_path)
		absolute_path = alloca(SB_PATH_MAX);

	if (!sb_abspathat(dirfd, file, absolute_path, SB_PATH_MAX))
		return 1;

	sb_debug_dyn("absolute_path: %s\n", absolute_path);

	resolved_path = rbuf = malloc(SB_PATH_MAX);
	if (!resolved_path)
		resolved_path = alloca(SB_PATH_MAX);

	if (!sb_realpathat(dirfd, file, resolved_path, SB_PATH_MAX,
				flags, is_create(sb_nr)))
		return 1;

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

	if (trace_dirfd >= 0)
		close(trace_dirfd);

	errno = old_errno;

	return result;

 error:
	/* The path is too long to be canonicalized, so just warn and let the
	 * function handle it (see bugs #21766 #94630 #101728 #227947)
	 */
	if (errno_is_too_long())
		return 2;

	/* Process went away while we were tracing it ... #264478 */
	if (trace_pid && errno == ESRCH)
		return 2;

	/* If we get here, something bad happened */
	sb_ebort("ISE: %s(%i, '%s')\n"
		"\tabs_path: %s\n"
		"\tres_path: %s\n"
		"\terrno=%i: %s\n",
		func, dirfd, file,
		absolute_path,
		resolved_path,
		errno, strerror(errno));
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

static bool reject_empty_path(int sb_nr, int flags)
{
	switch (sb_nr) {
		case SB_NR_ACCESS_RD:
		case SB_NR_ACCESS_WR:
		case SB_NR_FCHOWNAT:
		case SB_NR_FCHMODAT:
		case SB_NR_UTIMENSAT:
			return !(flags & AT_EMPTY_PATH);
		default:
			return true;
	}
}

static bool reject_null_path(int sb_nr)
{
	switch (sb_nr) {
		case SB_NR_FCHMOD:
		case SB_NR_FCHOWN:
		case SB_NR_FUTIMESAT:
		case SB_NR_UTIMENSAT:
			return false;
		default:
			return true;
	}
}

bool before_syscall(int sb_nr, const char *func, int dirfd, const char *file, int flags)
{
	int result;

	if (file == NULL) {
		/* futimesat treats dirfd as the target when file is NULL */
		if (reject_null_path(sb_nr))
			return true; /* let the kernel reject this */
	}
	else if (file[0] == '\0') {
		if (reject_empty_path(sb_nr, flags))
			return true; /* let the kernel reject this */
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

	result = check_syscall(&sbcontext, sb_nr, func, dirfd, file, flags);

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

bool before_syscall_access(int sb_nr, const char *func, int dirfd, const char *file, int mode, int flags)
{
	const char *ext_func;
	if (mode & W_OK) {
		sb_nr = SB_NR_ACCESS_WR;
		ext_func = "access_wr";
	}
	else if (mode & R_OK) {
		sb_nr = SB_NR_ACCESS_RD;
		ext_func = "access_rd";
	}
	else
		/* Must be F_OK or X_OK; we do not need to check either. */
		return true;
	return before_syscall(sb_nr, ext_func, dirfd, file, flags);
}

bool before_syscall_fd(int sb_nr, const char *func, int fd) {
	return before_syscall(sb_nr, func, fd, NULL, 0);
}

bool before_syscall_open_int(int sb_nr, const char *func, int dirfd, const char *file, int flags)
{
	if (flags & O_PATH)
		return true;

	const char *ext_func;
	if (flags & O_CREAT) {
		if (flags & (O_WRONLY|O_RDWR)) {
			sb_nr = SB_NR_OPEN_WR_CREAT;
			ext_func = "open_wr_creat";
		} else {
			sb_nr = SB_NR_OPEN_RD_CREAT;
			ext_func = "open_rd_creat";
		}
	}
	else if (flags & (O_WRONLY|O_RDWR)) {
		sb_nr = SB_NR_OPEN_WR;
		ext_func = "open_wr";
	} else {
		sb_nr = SB_NR_OPEN_RD;
		ext_func = "open_rd";
	}

	int atflags = 0;
	if (flags & O_NOFOLLOW)
		atflags |= AT_SYMLINK_NOFOLLOW;

	return before_syscall(sb_nr, ext_func, dirfd, file, atflags);
}

bool before_syscall_open_char(int sb_nr, const char *func, int dirfd, const char *file, const char *mode)
{
	if (NULL == mode)
		return false;

	const char *ext_func;
	if (mode[0] == 'r') {
		if (mode[1] == '+') {
			sb_nr = SB_NR_OPEN_WR;
			ext_func = "fopen_wr";
		} else {
			sb_nr = SB_NR_OPEN_RD;
			ext_func = "fopen_rd";
		}
	} else {
		sb_nr = SB_NR_OPEN_WR_CREAT;
		ext_func = "fopen_wr_creat";
	}
	return before_syscall(sb_nr, ext_func, dirfd, file, 0);
}

typedef struct {
	const char *name;
	size_t len;
	const char *value;
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
 *
 * execv*() must never modify environment inplace with
 * setenv/putenv/unsetenv as it can relocate 'environ' and break
 * vfork()/execv() users: https://bugs.gentoo.org/669702
 */
struct sb_envp_ctx sb_new_envp(char **envp, bool insert)
{
	struct sb_envp_ctx r = {
		.sb_envp   = envp,
		.orig_envp = envp,
		.__mod_cnt = 0,
	};
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
		ENV_PAIR(14, ENV_SANDBOX_METHOD, NULL),
	};
	size_t num_vars = ARRAY_SIZE(vars);
	char *found_vars[num_vars];
	size_t found_var_cnt;

	/* If sandbox is explicitly disabled, do not propagate the vars
	 * and just return user's envp */
	if (!sbcontext.on)
		return r;

	/* First figure out how many vars are already in the env */
	found_var_cnt = 0;
	memset(found_vars, 0, sizeof(found_vars));

	/* Iterate through user's environment and check against expected. */
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

	/* Treat unset and expected-unset variables as found. This will allow us
	 * to keep existing environment. */
	for (i = 0; i < num_vars; ++i) {
		if (vars[i].value == NULL && found_vars[i] == NULL) {
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
		return r;

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
	if (sbcontext.method != SANDBOX_METHOD_ANY)
		vars[14].value = str_sandbox_method(sbcontext.method);

	char ** my_env = NULL;
	if (!insert) {
		str_list_for_each_item(envp, entry, count) {
			for (i = 0; i < num_vars; ++i)
				if (i != 12 /* LD_LIBRARY_PATH index */
				    && is_env_var(entry, vars[i].name, vars[i].len)) {
					r.__mod_cnt++;
					goto skip;
				}
			str_list_add_item(my_env, entry, error);
 skip: ;
		}
	} else {
		/* Count directly due to variability with LD_PRELOAD and the value
		 * logic below.  Getting out of sync can mean memory corruption. */
		r.__mod_cnt = 0;
		if (unlikely(merge_ld_preload)) {
			str_list_add_item(my_env, ld_preload, error);
			r.__mod_cnt++;
		}
		for (i = 0; i < num_vars; ++i) {
			if (found_vars[i] || !vars[i].value)
				continue;
			str_list_add_item_env(my_env, vars[i].name, vars[i].value, error);
			r.__mod_cnt++;
		}

		str_list_for_each_item(envp, entry, count) {
			if (unlikely(merge_ld_preload && is_env_var(entry, vars[0].name, vars[0].len)))
				continue;
			str_list_add_item(my_env, entry, error);
		}
	}

 error:
	r.sb_envp = my_env;
	return r;
}

void sb_free_envp(struct sb_envp_ctx * envp_ctx)
{
	/* We assume all the stuffed vars are at the start */
	size_t mod_cnt = envp_ctx->__mod_cnt;
	char ** envp = envp_ctx->sb_envp;
	size_t i;
	for (i = 0; i < mod_cnt; ++i)
		free(envp[i]);

	/* We do not use str_list_free(), as we did not allocate the
	 * entries except for LD_PRELOAD.  All the other entries are
	 * pointers to existing envp memory.
	 */
	if (envp != envp_ctx->orig_envp)
		free(envp);
}
