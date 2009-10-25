/* Limited sandbox support for static binaries
 *
 * Copyright 2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "libsandbox.h"
#include "wrappers.h"
#include "sb_nr.h"

static long _do_ptrace(enum __ptrace_request request, const char *srequest, void *addr, void *data);
#define do_ptrace(request, addr, data) _do_ptrace(request, #request, addr, data)

#ifdef DEBUG
# define SBDEBUG 1
#else
# define SBDEBUG 0
#endif
#define __SB_DEBUG(fmt, args...) do { if (SBDEBUG) sb_printf(fmt, ## args); } while (0)
#define _SB_DEBUG(fmt, args...)  do { if (SBDEBUG) SB_EWARN("TRACE ", "(pid=%i):%s: " fmt, getpid(), __func__, ## args); } while (0)
#define SB_DEBUG(fmt, args...)   _SB_DEBUG(fmt "\n", ## args)

#include "trace/os.c"

pid_t trace_pid;

#ifndef SB_NO_TRACE

#ifdef HAVE_OPEN64
# define sb_openat_pre_check sb_openat64_pre_check
#endif

static void trace_exit(int status)
{
	/* if we were vfork-ed, clear trace_pid and exit */
	trace_pid = 0;
	_exit(status);
}

static long _do_ptrace(enum __ptrace_request request, const char *srequest, void *addr, void *data)
{
	long ret;
 try_again:
	errno = 0;
	ret = ptrace(request, trace_pid, addr, data);
	if (ret == -1) {
		/* Child hasn't gotten to the next marker yet ? */
		if (errno == ESRCH) {
			int status;
			if (waitpid(trace_pid, &status, 0) == -1) {
				/* nah, it's dead ... should we whine though ? */
				trace_exit(0);
			}
			sched_yield();
			goto try_again;
		} else if (!errno)
			if (request == PTRACE_PEEKDATA ||
			    request == PTRACE_PEEKTEXT ||
			    request == PTRACE_PEEKUSER)
				return ret;

		SB_EERROR("ISE:_do_ptrace ", "ptrace(%s, ..., %p, %p): %s\n",
			srequest, addr, data, strerror(errno));
		sb_abort();
	}
	return ret;
}

static long do_peekuser(long offset)
{
	return do_ptrace(PTRACE_PEEKUSER, (void *)offset, NULL);
}

static long do_peekdata(long offset)
{
	return do_ptrace(PTRACE_PEEKDATA, (void *)offset, NULL);
}

static char *do_peekstr(unsigned long lptr)
{
	size_t len, l;
	char *ret;
	long a, i;
	union {
		long val;
		char x[sizeof(long)];
	} s;

	/* if someone does open(NULL), don't shit a brick over it */
	if (lptr < sizeof(long))
		return NULL;

	l = 0;
	len = 1024;
	ret = xmalloc(len);
	while (1) {
		a = lptr & (sizeof(long) - 1);
		lptr -= a;
		s.val = do_peekdata(lptr);
		for (i = a; i < sizeof(long); ++i) {
			ret[l++] = s.x[i];
			if (!s.x[i])
				return ret;
			if (l >= len) {
				len += 1024;
				ret = xrealloc(ret, len);
			}
		}
		lptr += sizeof(long);
	}
}

static const char *strcld_chld(int cld)
{
	switch (cld) {
#define C(c) case CLD_##c: return "CLD_"#c;
	C(CONTINUED)
	C(DUMPED)
	C(EXITED)
	C(KILLED)
	C(TRAPPED)
	C(STOPPED)
#undef C
	default: return "CLD_???";
	}
}
/* strsignal() translates the string when i want C define */
static const char *strsig(int sig)
{
	switch (sig) {
#define S(s) case SIG##s: return "SIG"#s;
	S(ABRT)
	S(ALRM)
	S(CHLD)
	S(CONT)
	S(HUP)
	S(INT)
	S(KILL)
	S(QUIT)
	S(SEGV)
	S(STOP)
	S(TERM)
	S(TRAP)
#undef S
	default: return "SIG???";
	}
}

static void trace_child_signal(int signo, siginfo_t *info, void *context)
{
	SB_DEBUG("got sig %s(%i): code:%s(%i) status:%s(%i)",
		strsig(signo), signo,
		strcld_chld(info->si_code), info->si_code,
		strsig(info->si_status), info->si_status);

	switch (info->si_code) {
		case CLD_DUMPED:
		case CLD_KILLED:
			trace_exit(128 + info->si_status);

		case CLD_EXITED:
			__SB_DEBUG(" = %i\n", info->si_status);
			trace_exit(info->si_status);

		case CLD_TRAPPED:
			switch (info->si_status) {
				case SIGSTOP:
					kill(trace_pid, SIGCONT);
				case SIGTRAP:
				case SIGCHLD:
				case SIGCONT:
					return;
			}

			/* For whatever signal the child caught, let's ignore it and
			 * continue on.  If it aborted, segfaulted, whatever, that's
			 * its problem, not ours, so don't whine about it.  We just
			 * have to be sure to bubble it back up.  #265072
			 */
			do_ptrace(PTRACE_CONT, NULL, (void *)(long)info->si_status);
			return;
	}

	SB_EERROR("ISE:trace_child_signal ", "child (%i) signal %s(%i), code %s(%i), status %s(%i)\n",
		trace_pid,
		strsig(signo), signo,
		strcld_chld(info->si_code), info->si_code,
		strsig(info->si_status), info->si_status);
}

static const struct syscall_entry *lookup_syscall_in_tbl(const struct syscall_entry *tbl, int nr)
{
	while (tbl->name)
		if (tbl->nr == nr)
			return tbl;
		else
			++tbl;
	return NULL;
}
static const struct syscall_entry *lookup_syscall(int nr)
{
	return lookup_syscall_in_tbl(trace_check_personality(), nr);
}

struct syscall_state {
	void *regs;
	int nr;
	const char *func;
	bool (*pre_check)(const char *func, const char *pathname, int dirfd);
};

static bool _trace_check_syscall_C(struct syscall_state *state, int ibase)
{
	char *path = do_peekstr(trace_arg(state->regs, ibase));
	__SB_DEBUG("(\"%s\")", path);
	bool pre_ret, ret;
	if (state->pre_check)
		pre_ret = state->pre_check(state->func, path, AT_FDCWD);
	else
		pre_ret = true;
	if (pre_ret)
		ret = _SB_SAFE(state->nr, state->func, path);
	else
		ret = true;
	free(path);
	return ret;
}
static bool trace_check_syscall_C(struct syscall_state *state)
{
	return _trace_check_syscall_C(state, 1);
}

static bool __trace_check_syscall_DCF(struct syscall_state *state, int ibase, int flags)
{
	int dirfd = trace_arg(state->regs, ibase);
	char *path = do_peekstr(trace_arg(state->regs, ibase + 1));
	__SB_DEBUG("(%i, \"%s\", %x)", dirfd, path, flags);
	bool pre_ret, ret;
	if (state->pre_check)
		pre_ret = state->pre_check(state->func, path, dirfd);
	else
		pre_ret = true;
	if (pre_ret)
		ret = _SB_SAFE_AT(state->nr, state->func, dirfd, path, flags);
	else
		ret = true;
	free(path);
	return ret;
}
static bool _trace_check_syscall_DCF(struct syscall_state *state, int ibase)
{
	int flags = trace_arg(state->regs, ibase + 2);
	return __trace_check_syscall_DCF(state, ibase, flags);
}
static bool trace_check_syscall_DCF(struct syscall_state *state)
{
	return _trace_check_syscall_DCF(state, 1);
}

static bool _trace_check_syscall_DC(struct syscall_state *state, int ibase)
{
	return __trace_check_syscall_DCF(state, ibase, 0);
}
static bool trace_check_syscall_DC(struct syscall_state *state)
{
	return _trace_check_syscall_DC(state, 1);
}

static bool trace_check_syscall(const struct syscall_entry *se, void *regs)
{
	struct syscall_state state;
	bool ret = true;
	int nr;
	const char *name;

	/* These funcs aren't syscalls and so there are no checks:
	 *  - fopen
	 *  - lutimes
	 *  - mkfifo*
	 *  - opendir (uses open())
	 *  - _*x*mknod*
	 *  - 64bit versions of most funcs
	 *  - system / popen / most exec funcs
	 *
	 * Can't use switch() statement here as we auto define missing
	 * SYS_xxx to SB_NR_UNDEF in the build system
	 */
	if (!se)
		goto done;

	state.regs = regs;
	state.nr = nr = se->sys;
	state.func = name = se->name;
	if (nr == SB_NR_UNDEF)          goto done;
	else if (nr == SB_NR_MKDIR)     state.pre_check = sb_mkdirat_pre_check;
	else if (nr == SB_NR_MKDIRAT)   state.pre_check = sb_mkdirat_pre_check;
	else if (nr == SB_NR_UNLINK)    state.pre_check = sb_unlinkat_pre_check;
	else if (nr == SB_NR_UNLINKAT)  state.pre_check = sb_unlinkat_pre_check;
	else                            state.pre_check = NULL;

	/* Hmm, add these functions to the syscall table and avoid this if() ? */
	if (nr == SB_NR_UNDEF)          goto done;
	else if (nr == SB_NR_CHMOD)     return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_CHOWN)     return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_CREAT)     return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_FCHMODAT)  return  trace_check_syscall_DCF(&state);
	else if (nr == SB_NR_FCHOWNAT)  return  trace_check_syscall_DCF(&state);
	else if (nr == SB_NR_FUTIMESAT) return  trace_check_syscall_DC (&state);
	else if (nr == SB_NR_LCHOWN)    return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_LINK)      return _trace_check_syscall_C  (&state, 2);
	else if (nr == SB_NR_LINKAT)    return _trace_check_syscall_DCF(&state, 3);
	else if (nr == SB_NR_MKDIR)     return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_MKDIRAT)   return  trace_check_syscall_DC (&state);
	else if (nr == SB_NR_MKNOD)     return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_MKNODAT)   return  trace_check_syscall_DC (&state);
	else if (nr == SB_NR_RENAME)    return  trace_check_syscall_C  (&state) &&
	                                       _trace_check_syscall_C  (&state, 2);
	else if (nr == SB_NR_RENAMEAT)  return  trace_check_syscall_DC (&state) &&
	                                       _trace_check_syscall_DC (&state, 3);
	else if (nr == SB_NR_RMDIR)     return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_SYMLINK)   return _trace_check_syscall_C  (&state, 2);
	else if (nr == SB_NR_SYMLINKAT) return _trace_check_syscall_DC (&state, 2);
	else if (nr == SB_NR_TRUNCATE)  return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_TRUNCATE64)return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_UNLINK)    return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_UNLINKAT)  return  trace_check_syscall_DCF(&state);
	else if (nr == SB_NR_UTIME)     return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_UTIMES)    return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_UTIMENSAT) return _trace_check_syscall_DCF(&state, 1);

	else if (nr == SB_NR_ACCESS) {
		char *path = do_peekstr(trace_arg(regs, 1));
		int flags = trace_arg(regs, 2);
		__SB_DEBUG("(\"%s\", %x)", path, flags);
		ret = _SB_SAFE_ACCESS(nr, name, path, flags);
		free(path);
		return ret;

	} else if (nr == SB_NR_FACCESSAT) {
		int dirfd = trace_arg(regs, 1);
		char *path = do_peekstr(trace_arg(regs, 2));
		int flags = trace_arg(regs, 3);
		__SB_DEBUG("(%i, \"%s\", %x)", dirfd, path, flags);
		ret = _SB_SAFE_ACCESS_AT(nr, name, dirfd, path, flags);
		free(path);
		return ret;

	} else if (nr == SB_NR_OPEN) {
		char *path = do_peekstr(trace_arg(regs, 1));
		int flags = trace_arg(regs, 2);
		__SB_DEBUG("(\"%s\", %x)", path, flags);
		if (sb_openat_pre_check(name, path, AT_FDCWD, flags))
			ret = _SB_SAFE_OPEN_INT(nr, name, path, flags);
		else
			ret = 1;
		free(path);
		return ret;

	} else if (nr == SB_NR_OPENAT) {
		int dirfd = trace_arg(regs, 1);
		char *path = do_peekstr(trace_arg(regs, 2));
		int flags = trace_arg(regs, 3);
		__SB_DEBUG("(%i, \"%s\", %x)", dirfd, path, flags);
		if (sb_openat_pre_check(name, path, dirfd, flags))
			ret = _SB_SAFE_OPEN_INT_AT(nr, name, dirfd, path, flags);
		else
			ret = 1;
		free(path);
		return ret;
	}

 done:
	__SB_DEBUG("(...)");
	return ret;
}

/* Some arches (like hppa) don't implement PTRACE_GETREGS, while others (like
 * sparc) swap the meaning of "addr" and "data.  What a bunch of asshats.
 */
#ifndef trace_get_regs
# define trace_get_regs(regs) do_ptrace(PTRACE_GETREGS, NULL, regs)
#endif
/* Some arches (like sparc) don't implement PTRACE_PEEK* ...
 * more asshats !
 */
#ifndef trace_sysnum_regs
# define trace_sysnum_regs(regs) trace_sysnum()
#endif

static void trace_loop(void)
{
	trace_regs regs;
	bool before_syscall;
	long ret;
	int nr, exec_state;
	const struct syscall_entry *se, *tbl_at_fork;

	exec_state = 0;
	before_syscall = true;
	tbl_at_fork = NULL;
	do {
		ret = do_ptrace(PTRACE_SYSCALL, NULL, NULL);
		waitpid(trace_pid, NULL, 0);
		nr = trace_sysnum_regs(&regs);

		if (!exec_state) {
			if (!tbl_at_fork)
				tbl_at_fork = trace_check_personality();
			se = lookup_syscall_in_tbl(tbl_at_fork, nr);
			if (!before_syscall || !se || se->sys != SB_NR_EXECVE) {
				if (before_syscall)
					_SB_DEBUG(">%s:%i", se ? se->name : "IDK", nr);
				else
					__SB_DEBUG("(...pre-exec...) = ...\n");
				goto loop_again;
			}
			++exec_state;
		} else if (exec_state == 1) {
			/* Don't bother poking exec return */
			++exec_state;
			goto loop_again;
		}

		se = lookup_syscall(nr);
		ret = trace_get_regs(&regs);
		if (before_syscall) {
			_SB_DEBUG("%s:%i", se ? se->name : "IDK", nr);
			if (!trace_check_syscall(se, &regs)) {
				if (is_env_on(ENV_SANDBOX_DEBUG))
					SB_EINFO("trace_loop", " destroying after %s\n",
						se->name);
				do_ptrace(PTRACE_KILL, NULL, NULL);
				exit(1);
			}
		} else {
			int err;
			ret = trace_result(&regs, &err);

			__SB_DEBUG(" = %li", ret);
			if (err) {
				__SB_DEBUG(" (errno: %i: %s)", err, strerror(err));

				/* If the exec() failed for whatever reason, kill the
				 * child and have the parent resume like normal
				 */
				if (exec_state == 1) {
					do_ptrace(PTRACE_KILL, NULL, NULL);
					trace_pid = 0;
					return;
				}
			}
			__SB_DEBUG("\n");

			exec_state = 2;
		}

 loop_again:
		before_syscall = !before_syscall;
	} while (1);
}

void trace_main(const char *filename, char *const argv[])
{
	struct sigaction sa, old_sa;

	sa.sa_flags = SA_RESTART | SA_SIGINFO;
	sa.sa_sigaction = trace_child_signal;
	sigaction(SIGCHLD, &sa, &old_sa);

	if (is_env_on(ENV_SANDBOX_DEBUG))
		SB_EINFO("trace_main", " tracing: %s\n", filename);

	if (trace_pid) {
		SB_EERROR("ISE:trace_main ", "trace code assumes multiple threads are not forking\n");
		sb_abort();
	}

	trace_pid = fork();
	if (trace_pid == -1) {
		SB_EERROR("ISE:trace_main ", "vfork() failed: %s\n",
			strerror(errno));
		sb_abort();
	} else if (trace_pid) {
		SB_DEBUG("parent waiting for child (pid=%i) to signal", trace_pid);
		waitpid(trace_pid, NULL, 0);
#if defined(PTRACE_SETOPTIONS) && defined(PTRACE_O_TRACESYSGOOD)
		/* Not all kernel versions support this, so ignore return */
		ptrace(PTRACE_SETOPTIONS, trace_pid, NULL, (void *)PTRACE_O_TRACESYSGOOD);
#endif
		trace_loop();
		return;
	}

	SB_DEBUG("child setting up ...");
	sigaction(SIGCHLD, &old_sa, NULL);
	do_ptrace(PTRACE_TRACEME, NULL, NULL);
	kill(getpid(), SIGSTOP);
	/* child returns */
}

#else

static char *flatten_args(char *const argv[])
{
	char *ret;
	size_t i, len;

	len = 1;
	for (i = 0; argv[i]; ++i) {
		len += strlen(argv[i]) + 1;
		if (strchr(argv[i], ' '))
			len += 2;
	}

	ret = xmalloc(len);
	ret[0] = '\0';
	for (i = 0; argv[i]; ++i) {
		if (strchr(argv[i], ' ')) {
			strcat(ret, "'");
			strcat(ret, argv[i]);
			strcat(ret, "'");
		} else
			strcat(ret, argv[i]);
		strcat(ret, " ");
	}

	return ret;
}

void trace_main(const char *filename, char *const argv[])
{
	char *args = flatten_args(argv);
	sb_eqawarn("Static ELF: %s: %s\n", filename, args);
	free(args);
}

#endif
