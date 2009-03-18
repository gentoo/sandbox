/* Limited sandbox support for static binaries
 *
 * Copyright 2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"
#include "libsandbox.h"
#include "wrappers.h"
#include "sb_nr.h"

#include "trace/os.c"

pid_t trace_pid;

#ifndef SB_NO_TRACE

#ifdef DEBUG
# define SBDEBUG 1
#else
# define SBDEBUG 0
#endif
#define __SB_DEBUG(fmt, args...) do { if (SBDEBUG) sb_printf(fmt, ## args); } while (0)
#define _SB_DEBUG(fmt, args...)  do { if (SBDEBUG) SB_EWARN("TRACE ", "(pid=%i):%s: " fmt, getpid(), __func__, ## args); } while (0)
#define SB_DEBUG(fmt, args...)   _SB_DEBUG(fmt "\n", ## args)

static volatile bool child_stopped;

static long _do_ptrace(enum __ptrace_request request, const char *srequest, void *addr, void *data)
{
	long ret;
	errno = 0;
 try_again:
	ret = ptrace(request, trace_pid, addr, data);
	if (ret == -1 || (request == PTRACE_PEEKUSER && errno)) {
		/* Child hasn't gotten to the next marker yet */
		if (errno == ESRCH) {
			sched_yield();
			goto try_again;
		}

		SB_EERROR("ISE:trace_loop ", "ptrace(%s): %s\n",
			srequest, strerror(errno));
		sb_abort();
	}
	return ret;
}
#define do_ptrace(request, addr, data) _do_ptrace(request, #request, addr, data)

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

static void trace_child_signal(int signo, siginfo_t *info, void *context)
{
	SB_DEBUG("got sig %i: code:%i status:%i", signo, info->si_code, info->si_status);

	switch (info->si_code) {
		case CLD_KILLED:
			exit(128 + info->si_status);

		case CLD_EXITED:
			__SB_DEBUG(" = %i\n", info->si_status);
			exit(info->si_status);

		case CLD_TRAPPED:
			switch (info->si_status) {
				case SIGSTOP:
					kill(trace_pid, SIGCONT);
				case SIGTRAP:
					child_stopped = true;
				case SIGCONT:
					return;
			}
			/* fall through */
	}

	SB_EERROR("ISE:trace_child_signal ", "child signal %i, code %i, status %i\n",
		signo, info->si_code, info->si_status);
	sb_abort();
}

static const struct {
	const int nr;
	const char *name;
} syscalls[] = {
#define S(s) { SYS_##s, #s },
#include "trace.h"
#undef S
};
static const char *sysname(int nr)
{
	size_t i;
	for (i = 0; i < ARRAY_SIZE(syscalls); ++i)
		if (syscalls[i].nr == nr)
			return syscalls[i].name;
	return "unk";
}

static bool _trace_check_syscall_C(void *vregs, int sb_nr, const char *func, int ibase)
{
	char *path = do_peekstr(trace_arg(vregs, ibase));
	__SB_DEBUG("(\"%s\")", path);
	bool ret = _SB_SAFE(sb_nr, func, path);
	free(path);
	return ret;
}
static bool trace_check_syscall_C(void *vregs, int sb_nr, const char *func)
{
	return _trace_check_syscall_C(vregs, sb_nr, func, 1);
}

static bool __trace_check_syscall_DCF(void *vregs, int sb_nr, const char *func, int ibase, int flags)
{
	int dirfd = trace_arg(vregs, ibase);
	char *path = do_peekstr(trace_arg(vregs, ibase + 1));
	__SB_DEBUG("(%i, \"%s\", %x)", dirfd, path, flags);
	bool ret = _SB_SAFE_AT(sb_nr, func, dirfd, path, flags);
	free(path);
	return ret;
}
static bool _trace_check_syscall_DCF(void *vregs, int sb_nr, const char *func, int ibase)
{
	int flags = trace_arg(vregs, ibase + 2);
	return __trace_check_syscall_DCF(vregs, sb_nr, func, ibase, flags);
}
static bool trace_check_syscall_DCF(void *vregs, int sb_nr, const char *func)
{
	return _trace_check_syscall_DCF(vregs, sb_nr, func, 1);
}

static bool _trace_check_syscall_DC(void *vregs, int sb_nr, const char *func, int ibase)
{
	return __trace_check_syscall_DCF(vregs, sb_nr, func, ibase, 0);
}
static bool trace_check_syscall_DC(void *vregs, int sb_nr, const char *func)
{
	return _trace_check_syscall_DC(vregs, sb_nr, func, 1);
}

static bool trace_check_syscall(int nr, void *vregs)
{
	struct user_regs_struct *regs = vregs;
	bool ret = true;

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
	if (nr == SB_NR_UNDEF)
		goto done;
	else if (nr == SYS_chmod)     return  trace_check_syscall_C  (vregs, SB_NR_CHMOD, "chmod");
	else if (nr == SYS_chown)     return  trace_check_syscall_C  (vregs, SB_NR_CHOWN, "chown");
	else if (nr == SYS_creat)     return  trace_check_syscall_C  (vregs, SB_NR_CREAT, "creat");
	else if (nr == SYS_fchmodat)  return  trace_check_syscall_DCF(vregs, SB_NR_FCHMODAT, "fchmodat");
	else if (nr == SYS_fchownat)  return  trace_check_syscall_DCF(vregs, SB_NR_FCHOWNAT, "fchownat");
	else if (nr == SYS_futimesat) return  trace_check_syscall_DC (vregs, SB_NR_FUTIMESAT, "futimesat");
	else if (nr == SYS_lchown)    return  trace_check_syscall_C  (vregs, SB_NR_LCHOWN, "lchown");
	else if (nr == SYS_link)      return _trace_check_syscall_C  (vregs, SB_NR_LINK, "link", 2);
	else if (nr == SYS_linkat)    return _trace_check_syscall_DCF(vregs, SB_NR_LINKAT, "linkat", 3);
	else if (nr == SYS_mkdir)     return  trace_check_syscall_C  (vregs, SB_NR_MKDIR, "mkdir");
	else if (nr == SYS_mkdirat)   return  trace_check_syscall_DC (vregs, SB_NR_MKDIRAT, "mkdirat");
	else if (nr == SYS_mknod)     return  trace_check_syscall_C  (vregs, SB_NR_MKNOD, "mknod");
	else if (nr == SYS_mknodat)   return  trace_check_syscall_DC (vregs, SB_NR_MKNODAT, "mknodat");
	else if (nr == SYS_rename)    return  trace_check_syscall_C  (vregs, SB_NR_RENAME, "rename") &&
	                                     _trace_check_syscall_C  (vregs, SB_NR_RENAME, "rename", 2);
	else if (nr == SYS_renameat)  return  trace_check_syscall_DC (vregs, SB_NR_RENAMEAT, "renameat") &&
	                                     _trace_check_syscall_DC (vregs, SB_NR_RENAMEAT, "renameat", 3);
	else if (nr == SYS_rmdir)     return  trace_check_syscall_C  (vregs, SB_NR_RMDIR, "rmdir");
	else if (nr == SYS_symlink)   return _trace_check_syscall_C  (vregs, SB_NR_SYMLINK, "symlink", 2);
	else if (nr == SYS_symlinkat) return _trace_check_syscall_DC (vregs, SB_NR_SYMLINKAT, "symlinkat", 2);
	else if (nr == SYS_truncate)  return  trace_check_syscall_C  (vregs, SB_NR_TRUNCATE, "truncate");
	else if (nr == SYS_truncate64)return  trace_check_syscall_C  (vregs, SB_NR_TRUNCATE64, "truncate64");
	else if (nr == SYS_unlink)    return  trace_check_syscall_C  (vregs, SB_NR_UNLINK, "unlink");
	else if (nr == SYS_unlinkat)  return  trace_check_syscall_DCF(vregs, SB_NR_UNLINKAT, "unlinkat");
	else if (nr == SYS_utime)     return  trace_check_syscall_C  (vregs, SB_NR_UTIME, "utime");
	else if (nr == SYS_utimes)    return  trace_check_syscall_C  (vregs, SB_NR_UTIMES, "utimes");
	else if (nr == SYS_utimensat) return  trace_check_syscall_DCF(vregs, SB_NR_UTIMENSAT, "utimensat");

	else if (nr == SYS_access) {
		char *path = do_peekstr(trace_arg(regs, 1));
		int flags = trace_arg(regs, 2);
		__SB_DEBUG("(\"%s\", %x)", path, flags);
		ret = _SB_SAFE_ACCESS(SB_NR_ACCESS, "access", path, flags);
		free(path);
		return ret;

	} else if (nr == SYS_faccessat) {
		int dirfd = trace_arg(regs, 1);
		char *path = do_peekstr(trace_arg(regs, 2));
		int flags = trace_arg(regs, 3);
		__SB_DEBUG("(%i, \"%s\", %x)", dirfd, path, flags);
		ret = _SB_SAFE_ACCESS_AT(SB_NR_FACCESSAT, "faccessat", dirfd, path, flags);
		free(path);
		return ret;

	} else if (nr == SYS_open) {
		char *path = do_peekstr(trace_arg(regs, 1));
		int flags = trace_arg(regs, 2);
		__SB_DEBUG("(\"%s\", %x)", path, flags);
		ret = _SB_SAFE_OPEN_INT(SB_NR_OPEN, "open", path, flags);
		free(path);
		return ret;

	} else if (nr == SYS_openat) {
		int dirfd = trace_arg(regs, 1);
		char *path = do_peekstr(trace_arg(regs, 2));
		int flags = trace_arg(regs, 3);
		__SB_DEBUG("(%i, \"%s\", %x)", dirfd, path, flags);
		ret = _SB_SAFE_OPEN_INT_AT(SB_NR_OPENAT, "openat", dirfd, path, flags);
		free(path);
		return ret;
	}

 done:
	__SB_DEBUG("(...)");
	return ret;
}

static void trace_loop(void)
{
	struct user_regs_struct regs;
	bool before_syscall;
	long ret;
	int nr, exec_state;

	exec_state = 0;
	before_syscall = true;
	do {
		ret = do_ptrace(PTRACE_SYSCALL, NULL, NULL);
		nr = trace_sysnum();
		if (!exec_state) {
			if (!before_syscall || nr != SYS_execve)
				goto loop_again;
			++exec_state;
		}

		ret = do_ptrace(PTRACE_GETREGS, NULL, &regs);
		if (before_syscall) {
			_SB_DEBUG("%s:%i", sysname(nr), nr);
			if (!trace_check_syscall(nr, &regs)) {
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

	child_stopped = false;
	sa.sa_flags = SA_RESTART | SA_SIGINFO;
	sa.sa_sigaction = trace_child_signal;
	sigaction(SIGCHLD, &sa, &old_sa);

	trace_pid = fork();
	if (trace_pid == -1) {
		SB_EERROR("ISE:trace_main ", "vfork() failed: %s\n",
			strerror(errno));
		sb_abort();
	} else if (trace_pid) {
		SB_DEBUG("parent waiting for child (pid=%i) to signal", trace_pid);
		while (!child_stopped)
			sched_yield();
#ifdef PTRACE_O_TRACESYSGOOD
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
