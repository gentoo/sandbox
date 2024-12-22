/* Limited sandbox support for static binaries
 *
 * Copyright 2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "libsandbox.h"
#include "wrappers.h"
#include "sb_nr.h"

static long do_peekdata(long offset);
/* Note on _do_ptrace argument types:
   glibc defines ptrace as:
     long ptrace(enum __ptrace_request request, pid_t pid, void *addr, void *data);
   musl defines ptrace as:
     long ptrace(int, ...);

   Let's clobber to 'int' lowest common denominator.
 */
typedef int sb_ptrace_req_t;
static long _do_ptrace(sb_ptrace_req_t request, const char *srequest, void *addr, void *data);
#define do_ptrace(request, addr, data) _do_ptrace(request, #request, addr, data)
#define _trace_possible(data) true

#ifdef DEBUG
# define SBDEBUG 1
#else
# define SBDEBUG 0
#endif
#define __sb_debug(fmt, args...) do { if (SBDEBUG) sb_eraw(fmt, ## args); } while (0)
#define _sb_debug(fmt, args...)  do { if (SBDEBUG) sb_ewarn("TRACE (pid=%i<%i):%s: " fmt, getpid(), trace_pid, __func__, ## args); } while (0)
#define sb_debug(fmt, args...)   _sb_debug(fmt "\n", ## args)

#include "trace/os.c"

pid_t trace_pid;

#ifndef SB_NO_TRACE

#ifndef HAVE_TRACE_REGS
# error "unable to find struct for tracing regs"
#endif

#ifdef HAVE_OPEN64
# define sb_openat_pre_check sb_openat64_pre_check
#endif

static int trace_yama_level(void)
{
	char ch;
	int fd, level;

	fd = open64("/proc/sys/kernel/yama/ptrace_scope", O_RDONLY | O_CLOEXEC);
	if (fd == -1)
		return 0;

	RETRY_EINTR(read(fd, &ch, 1));
	close(fd);
	level = ch - '0';

	switch (level) {
	case 0:
		/* Normal levels work fine. */
		return 0;

	case 1:
	case 2:
		/* ptrace scope binds access to specific capabilities.  Lets use uid==0 as a
		 * lazy proxy for "we have all capabilities" until we can refine this.
		 */
		return getuid() == 0 ? 0 : level;

	case 3:
	default:
		/* Level 3+ is not supported. */
		sb_ebort("YAMA ptrace_scope=%i+ is not supported as it makes tracing impossible.\n", level);
	}
}

static void trace_exit(int status)
{
	/* if we were vfork-ed, clear trace_pid and exit */
	trace_pid = 0;
	_exit(status);
}

static long _do_ptrace(sb_ptrace_req_t request, const char *srequest, void *addr, void *data)
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
		} else if (errno == EIO || errno == EFAULT) {
			/* This comes up when the child itself tries to use a bad pointer.
			 * That's not something the sandbox should abort on. #560396
			 */
			return ret;
		} else if (!errno)
			if (request == PTRACE_PEEKDATA ||
			    request == PTRACE_PEEKTEXT ||
			    request == PTRACE_PEEKUSER)
				return ret;

		sb_ebort("ISE:_do_ptrace: ptrace(%s, ..., %p, %p): %s\n",
			srequest, addr, data, strerror(errno));
	}
	return ret;
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

#ifdef HAVE_PROCESS_VM_READV
	struct iovec liov, riov;

	/* We can't cross remote page boundaries when using this :(.
	 * The first call reads up to the edge of the page boundary since we
	 * have no (easy) way of knowing if the next page is valid.  This is
	 * less effort than reading in the /proc/.../maps file and walking it.
	 */
	l = 0x1000;
	riov.iov_base = (void *)lptr;
	len = l - (lptr % l);
	if (!len)
		len = l;
	liov.iov_base = ret = xmalloc(len);
	riov.iov_len = liov.iov_len = len;

	while (1) {
		if (process_vm_readv(trace_pid, &liov, 1, &riov, 1, 0) == -1) {
			int e = errno;
			if (e == ENOSYS) {
				/* This can happen if run on older kernels but built with newer ones. */
				break;
			} else if (e == EFAULT) {
				/* This can happen if the target process uses a bad pointer. #560396 */
				break;
			}
			sb_ebort("ISE:do_peekstr:process_vm_readv(%i, %p{%p, %#zx}, 1, %p{%p, %#zx}, 1, 0) failed: %s\n",
				trace_pid,
				&liov, liov.iov_base, liov.iov_len,
				&riov, riov.iov_base, riov.iov_len,
				strerror(e));
		}

		if (memchr(liov.iov_base, '\0', liov.iov_len) != NULL)
			return ret;
		riov.iov_base += riov.iov_len;
		riov.iov_len = liov.iov_len = l;
		len += l;
		ret = xrealloc(ret, len);
		liov.iov_base = ret + len - l;
	}
#else
	len = 1024;
	ret = xmalloc(len);
#endif

	l = 0;
	while (1) {
		a = lptr & (sizeof(long) - 1);
		lptr -= a;
		errno = 0;
		s.val = do_peekdata(lptr);
		if (unlikely(errno)) {
			if (errno == EIO || errno == EFAULT) {
				ret[0] = '\0';
				return ret;
			}
			sb_ebort("ISE:do_peekstr:do_peekdata(%#lx) failed: %s\n",
				lptr, strerror(errno));
		}
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

static const struct syscall_entry *lookup_syscall_in_tbl(const struct syscall_entry *tbl, int nr)
{
	while (tbl->name)
		if (tbl->nr == nr)
			return tbl;
		else
			++tbl;
	return NULL;
}

struct syscall_state {
	void *regs;
	int nr;
	const char *func;
	bool (*pre_check)(const char *func, const char *pathname, int dirfd);
};

/* Check syscall that only takes a path as its |ibase| argument. */
static bool _trace_check_syscall_C(struct syscall_state *state, int ibase)
{
	char *path = do_peekstr(trace_arg(state->regs, ibase));
	__sb_debug("(\"%s\")", path);
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
/* Check syscall that only takes a path as its first argument. */
static bool trace_check_syscall_C(struct syscall_state *state)
{
	return _trace_check_syscall_C(state, 1);
}

static bool __trace_check_syscall_DCF(struct syscall_state *state, int ibase, int flags)
{
	int dirfd = trace_arg(state->regs, ibase);
	char *path = do_peekstr(trace_arg(state->regs, ibase + 1));
	__sb_debug("(%i, \"%s\", %x)", dirfd, path, flags);
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
/* Check syscall that takes a dirfd & path starting at |ibase| argument, and flags at |fbase|. */
static bool _trace_check_syscall_DCF(struct syscall_state *state, int ibase, int fbase)
{
	int flags = trace_arg(state->regs, fbase);
	return __trace_check_syscall_DCF(state, ibase, flags);
}
/* Check syscall that takes a dirfd, path, and flags as its first 3 arguments. */
static bool trace_check_syscall_DCF(struct syscall_state *state)
{
	return _trace_check_syscall_DCF(state, 1, 3);
}

/* Check syscall that takes a dirfd & path starting at |ibase| argument. */
static bool _trace_check_syscall_DC(struct syscall_state *state, int ibase)
{
	return __trace_check_syscall_DCF(state, ibase, 0);
}
/* Check syscall that takes a dirfd & path as its first 2 arguments (but no flags). */
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
	if (!SB_NR_IS_DEFINED(se->nr))  goto done;
	else if (nr == SB_NR_MKDIR)     state.pre_check = sb_mkdirat_pre_check;
	else if (nr == SB_NR_MKDIRAT)   state.pre_check = sb_mkdirat_pre_check;
	else if (nr == SB_NR_UNLINK)    state.pre_check = sb_unlinkat_pre_check;
	else if (nr == SB_NR_UNLINKAT)  state.pre_check = sb_unlinkat_pre_check;
	else                            state.pre_check = NULL;

	/* Hmm, add these functions to the syscall table and avoid this if() ? */
	     if (nr == SB_NR_CHMOD)     return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_CHOWN)     return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_CREAT)     return  trace_check_syscall_C  (&state);
	/* NB: Linux syscall does not have a flags argument. */
	else if (nr == SB_NR_FCHMODAT)  return  trace_check_syscall_DC (&state);
	else if (nr == SB_NR_FCHOWNAT)  return _trace_check_syscall_DCF(&state, 1, 5);
	else if (nr == SB_NR_FUTIMESAT) return  trace_check_syscall_DC (&state);
	else if (nr == SB_NR_LCHOWN)    return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_LINK)      return _trace_check_syscall_C  (&state, 2);
	else if (nr == SB_NR_LINKAT)    return _trace_check_syscall_DCF(&state, 3, 5);
	else if (nr == SB_NR_MKDIR)     return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_MKDIRAT)   return  trace_check_syscall_DC (&state);
	else if (nr == SB_NR_MKNOD)     return  trace_check_syscall_C  (&state);
	else if (nr == SB_NR_MKNODAT)   return  trace_check_syscall_DC (&state);
	else if (nr == SB_NR_RENAME)    return  trace_check_syscall_C  (&state) &&
	                                       _trace_check_syscall_C  (&state, 2);
	else if (nr == SB_NR_RENAMEAT2) return  trace_check_syscall_DC (&state) &&
	                                       _trace_check_syscall_DC (&state, 3);
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

	else if (nr == SB_NR_UTIMENSAT) {
 utimensat:
		return _trace_check_syscall_DCF(&state, 1, 4);
	} else if (nr == SB_NR___UTIMENSAT64) {
		state.nr = SB_NR_UTIMENSAT;
		goto utimensat;
	}

	else if (nr == SB_NR_ACCESS) {
		char *path = do_peekstr(trace_arg(regs, 1));
		int mode = trace_arg(regs, 2);
		__sb_debug("(\"%s\", %x)", path, mode);
		ret = _SB_SAFE_ACCESS(nr, name, path, mode);
		free(path);
		return ret;

	} else if (nr == SB_NR_FACCESSAT) {
		int dirfd = trace_arg(regs, 1);
		char *path = do_peekstr(trace_arg(regs, 2));
		int mode = trace_arg(regs, 3);
		__sb_debug("(%i, \"%s\", %x)", dirfd, path, mode);
		ret = _SB_SAFE_ACCESS_AT(nr, name, dirfd, path, mode, 0);
		free(path);
		return ret;

	} else if (nr == SB_NR_FACCESSAT2) {
		int dirfd = trace_arg(regs, 1);
		char *path = do_peekstr(trace_arg(regs, 2));
		int mode = trace_arg(regs, 3);
		int flags = trace_arg(regs, 4);
		__sb_debug("(%i, \"%s\", %x, %x)", dirfd, path, mode, flags);
		ret = _SB_SAFE_ACCESS_AT(nr, name, dirfd, path, mode, flags);
		free(path);
		return ret;

	} else if (nr == SB_NR_OPEN) {
		char *path = do_peekstr(trace_arg(regs, 1));
		int flags = trace_arg(regs, 2);
		__sb_debug("(\"%s\", %x)", path, flags);
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
		__sb_debug("(%i, \"%s\", %x)", dirfd, path, flags);
		if (sb_openat_pre_check(name, path, dirfd, flags))
			ret = _SB_SAFE_OPEN_INT_AT(nr, name, dirfd, path, flags);
		else
			ret = 1;
		free(path);
		return ret;

	} else if (nr == SB_NR_EXECVE || nr == SB_NR_EXECVEAT) {
		/* Try to extract environ and merge with our own. */
		char *path;
		unsigned long environ, i = 0;

		if (nr == SB_NR_EXECVEAT) {
			int dirfd = do_peekdata(trace_arg(regs, 1));
			unsigned long argv = trace_arg(regs, 3);
			environ = trace_arg(regs, 4);
			path = do_peekstr(trace_arg(regs, 2));
			__sb_debug("(%i, \"%s\", %lx, %lx{", dirfd, path, argv, environ);
		} else {
			path = do_peekstr(trace_arg(regs, 1));
			unsigned long argv = trace_arg(regs, 2);
			environ = trace_arg(regs, 3);
			__sb_debug("(\"%s\", %lx, %lx{", path, argv, environ);
		}

		while (1) {
			unsigned long envp = do_peekdata(environ + i);
			if (!envp)
				break;

			char *env = do_peekstr(envp);
			if (strncmp(env, "SANDBOX_", 8) == 0) {
				__sb_debug("\"%s\"  ", env);
				putenv(env);
			}
			i += sizeof(long);
		}
		__sb_debug("})");
		return 1;
	} else if (nr == SB_NR_FCHMOD) {
		int fd = trace_arg(regs, 1);
		mode_t mode = trace_arg(regs, 2);
		__sb_debug("(%i, %o)", fd, mode);
		return _SB_SAFE_FD(nr, name, fd);

	} else if (nr == SB_NR_FCHOWN) {
		int fd = trace_arg(regs, 1);
		uid_t uid = trace_arg(regs, 2);
		gid_t gid = trace_arg(regs, 3);
		__sb_debug("(%i, %i, %i)", fd, uid, gid);
		return _SB_SAFE_FD(nr, name, fd);
	}


 done:
	__sb_debug("(...)");
	return ret;
}

static void trace_init_tracee(void)
{
	do_ptrace(PTRACE_SETOPTIONS, NULL, (void *)(uintptr_t)(
		PTRACE_O_EXITKILL |
		PTRACE_O_TRACECLONE |
		PTRACE_O_TRACEEXEC |
		PTRACE_O_TRACEEXIT |
		PTRACE_O_TRACEFORK |
		PTRACE_O_TRACEVFORK |
		PTRACE_O_TRACESYSGOOD
	));
}

static void trace_loop(void)
{
	trace_regs regs;
	bool before_exec, before_syscall, fake_syscall_ret;
	unsigned event;
	long ret;
	int status, sig;
	const struct syscall_entry *tbl_after_fork;
	void *data;

	before_exec = true;
	before_syscall = false;
	fake_syscall_ret = false;
	tbl_after_fork = NULL;
	data = NULL;
	do {
		ret = do_ptrace(PTRACE_SYSCALL, NULL, data);
		data = NULL;
		waitpid(trace_pid, &status, 0);

		event = (unsigned)status >> 16;

		if (WIFSIGNALED(status)) {
			int sig = WTERMSIG(status);
			sb_debug("signaled %s %i", strsig(sig), sig);
			kill(getpid(), sig);
			trace_exit(128 + sig);

		} else if (WIFEXITED(status)) {
			ret = WEXITSTATUS(status);
			sb_debug("exited %li", ret);
			trace_exit(ret);

		}

		sb_assert(WIFSTOPPED(status));
		sig = WSTOPSIG(status);

		switch (event) {
		case 0:
			if (sig != (SIGTRAP | 0x80)) {
				/* For whatever signal the child caught, let's ignore it and
				 * continue on.  If it aborted, segfaulted, whatever, that's
				 * its problem, not ours, so don't whine about it.  We just
				 * have to be sure to bubble it back up.  #265072
				 *
				 * The next run of this loop should see the WIFSIGNALED status
				 * and we'll exit then.
				 */
				sb_debug("passing signal through %s (%i)", strsig(sig), sig);
				data = (void *)(uintptr_t)(sig);
				continue;
			}

			if (before_exec) {
				_sb_debug("waiting for exec; status: %#x", status);
				continue;
			}
			break;

		case PTRACE_EVENT_EXEC:
			__sb_debug("hit exec!");
			before_exec = false;
			ret = trace_get_regs(&regs);
			tbl_after_fork = trace_check_personality(&regs);
			continue;

		case PTRACE_EVENT_EXIT:
			/* We'll tell the process to resume, which should make it exit,
			 * and then we'll pick up its exit status and exit above.
			 */
			__sb_debug(" exit event!\n");
			continue;

		case PTRACE_EVENT_CLONE:
		case PTRACE_EVENT_FORK:
		case PTRACE_EVENT_VFORK: {
			/* The tracee is forking, so fork a new tracer to handle it. */
			long newpid;
			do_ptrace(PTRACE_GETEVENTMSG, NULL, &newpid);
			sb_debug("following forking event %i; pid=%li %i\n",
			         event, newpid, before_syscall);

			/* If YAMA ptrace_scope is active, then we can't hand off the child
			 * to a new tracer.  Give up.  #821403
			 */
			int yama = trace_yama_level();
			if (yama >= 1) {
				sb_eqawarn("Unable to trace children due to YAMA ptrace_scope=%i\n", yama);
				ptrace(PTRACE_DETACH, newpid, NULL, NULL);
				continue;
			}

			/* Pipe for synchronizing detach & attach events. */
			int fds[2];
			ret = pipe(fds);
			sb_assert(ret == 0);
			if (fork() == 0) {
				/* New tracer needs to take control of new tracee. */
				char ch;
				close(fds[1]);
				RETRY_EINTR(read(fds[0], &ch, 1));
				close(fds[0]);
				trace_pid = newpid;
 retry_attach:
				ret = do_ptrace(PTRACE_ATTACH, NULL, NULL);
				if (ret) {
					if (errno == EPERM)
						goto retry_attach;
					sb_ebort("ISE:PTRACE_ATTACH %s", strerror(errno));
				}
				trace_init_tracee();
				before_syscall = true;
				continue;
			} else {
				/* Existing tracer needs to release new tracee. */
 retry_detach:
				ret = ptrace(PTRACE_DETACH, newpid, NULL, (void *)SIGSTOP);
				if (ret) {
					if (errno == ESRCH) {
						/* The kernel might not have the proc ready yet. */
						struct timespec ts = {0, 500 * 1000 /* 0.5 millisec */};
						nanosleep(&ts, NULL);
						goto retry_detach;
					}
					sb_ebort("ISE:PTRACE_DETACH %s", strerror(errno));
				}
				close(fds[0]);
				RETRY_EINTR(write(fds[1], "", 1));
				close(fds[1]);
			}
			continue;
		}

		default:
			sb_ebort("ISE: unhandle ptrace signal %s (%i) event %u\n",
			         strsig(sig), sig, event);
		}

		ret = trace_get_regs(&regs);

		if (before_syscall) {
			/* NB: The kernel guarantees syscall NR is valid only on entry. */
			int nr = trace_get_sysnum(&regs);
			const struct syscall_entry *se = lookup_syscall_in_tbl(tbl_after_fork, nr);

			_sb_debug("%s:%i", se ? se->name : "IDK", nr);
			if (!trace_check_syscall(se, &regs)) {
				sb_debug_dyn("trace_loop: forcing EPERM after %s\n", se->name);
				trace_set_sysnum(&regs, -1);
				fake_syscall_ret = true;
			}
		} else {
			int err;

			if (unlikely(fake_syscall_ret)) {
				ret = -1;
				err = EPERM;
				trace_set_ret(&regs, err);
				fake_syscall_ret = false;
			} else
				ret = trace_result(&regs, &err);

			__sb_debug(" = %li", ret);
			if (err)
				__sb_debug(" (errno: %i: %s)", err, strerror(err));
			__sb_debug("\n");
		}

		before_syscall = !before_syscall;
	} while (1);
}

void trace_main(void)
{
	struct sigaction old_sa, sa = { .sa_handler = SIG_DFL, };

	if (trace_pid)
		sb_ebort("ISE: trace code assumes multiple threads are not forking\n");

	sigaction(SIGCHLD, &sa, &old_sa);
	trace_pid = fork();
	if (unlikely(trace_pid == -1)) {
		sb_ebort("ISE: vfork() failed: %s\n", strerror(errno));
	} else if (trace_pid) {
		sb_debug("parent waiting for child (pid=%i) to signal", trace_pid);
		waitpid(trace_pid, NULL, 0);
		trace_init_tracee();
		sb_close_all_fds();
		trace_loop();
		sb_ebort("ISE: child should have quit, as should we\n");
	}

	sb_debug("child setting up ...");
	sigaction(SIGCHLD, &old_sa, NULL);
	do_ptrace(PTRACE_TRACEME, NULL, NULL);
	kill(getpid(), SIGSTOP);
	/* child returns */
}

#else

#undef _trace_possible
#define _trace_possible(data) false

void trace_main(void)
{
	/* trace_possible() triggers a warning for us */
}

#endif

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

bool trace_possible(const char *filename, char *const argv[], const void *data)
{
	char *args;

	/* If YAMA ptrace_scope is very high, then we can't trace at all.  #771360 */
	int yama = trace_yama_level();
	if (yama >= 2) {
		sb_eqawarn("YAMA ptrace_scope=%i is not currently supported\n", yama);
		goto fail;
	}

	if (_trace_possible(data)) {
		/* If we're in an environment like QEMU where ptrace doesn't work, then
		 * don't try to use it.  If ptrace does work, this should fail with ESRCH.
		 */
		errno = 0;
		ptrace(PTRACE_CONT, 0, NULL, NULL);
		return errno == ENOSYS ? false : true;
	}

 fail:
	args = flatten_args(argv);
	sb_eqawarn("Unable to trace static ELF: %s: %s\n", filename, args);
	free(args);
	return false;
}
