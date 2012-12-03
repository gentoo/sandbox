/*
 * sb_efuncs.c
 *
 * Helpers for doing pretty output.
 *
 * Copyright 1999-2012 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

const char *colors[] = {
	"\033[0m",
	"\033[32;01m",
	"\033[33;01m",
	"\033[31;01m",
};
__attribute__((constructor))
static void sbio_init(void)
{
	if (is_env_on(ENV_NOCOLOR)) {
		size_t i;
		for (i = 0; i < ARRAY_SIZE(colors); ++i)
			colors[i] = "";
	}
}

static bool try_portage_helpers = true;

/*
 * First try to use the helper programs from portage so that it can sanely
 * log things itself, and so the output doesn't get consumed by something
 * else #278761.  If that fails, fall back to writing to /dev/tty.  While
 * this might annoy some people, using stderr will break tests that try to
 * validate output #261957.
 */
static void sb_vefunc(const char *prog, const char *color, const char *format, va_list args)
{
	char shellcode[128];
	FILE *fp;
	struct sigaction sa, old_sa;
	bool is_pipe = false;
	va_list retry_args;

	if (try_portage_helpers) {
		/* If popen() fails, then writes to it will trigger SIGPIPE */
		sa.sa_flags = SA_RESTART;
		sa.sa_handler = SIG_IGN;
		sigaction(SIGCHLD, &sa, &old_sa);

		sprintf(shellcode, "xargs %s 2>/dev/null", prog);
		fp = sbio_popen(shellcode, "we");
		is_pipe = true;
		va_copy(retry_args, args);
	} else
		fp = NULL;

	if (!fp) {
 do_tty:
		is_pipe = false;
		int fd = sbio_open(sbio_fallback_path, O_WRONLY|O_CLOEXEC, 0);
		if (fd >= 0)
			fp = fdopen(fd, "ae");
		if (!fp)
			fp = stderr;
	}

	sb_fprintf(fp, " %s*%s ", color, COLOR_NORMAL);
	sb_vfprintf(fp, format, args);

	if (is_pipe) {
		int status = pclose(fp);
		if (WEXITSTATUS(status)) {
			args = retry_args;
			goto do_tty;
		}
	} else if (fp != stderr)
		fclose(fp);

	if (try_portage_helpers) {
		sigaction(SIGCHLD, &old_sa, NULL);
		va_end(retry_args);
		if (!is_pipe)
			/* If we failed once, we'll fail again */
			try_portage_helpers = false;
	}
}

void sb_einfo(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	sb_vefunc("einfo", COLOR_GREEN, format, args);
	va_end(args);
}

void sb_debug_dyn(const char *format, ...)
{
	if (!is_env_on(ENV_SANDBOX_DEBUG))
		return;

	va_list args;
	va_start(args, format);
	sb_vefunc("einfo", COLOR_GREEN, format, args);
	va_end(args);
}

void sb_ewarn(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	sb_vefunc("ewarn", COLOR_YELLOW, format, args);
	va_end(args);
}

void sb_eerror(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	sb_vefunc("eerror", COLOR_RED, format, args);
	va_end(args);
}

void sb_eqawarn(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	sb_vefunc("eqawarn", COLOR_YELLOW, format, args);
	va_end(args);
}

void sb_dump_backtrace(void)
{
#ifdef HAVE_BACKTRACE
	void *funcs[10];
	int num_funcs;
	num_funcs = backtrace(funcs, ARRAY_SIZE(funcs));
	backtrace_symbols_fd(funcs, num_funcs, STDERR_FILENO);
#endif
	__sb_dump_backtrace();
}

void __sb_ebort(const char *file, const char *func, size_t line_num, const char *format, ...)
{
	va_list args;

	sb_eerror("%s:%s():%zu: failure (%s):\n", file, func, line_num, strerror(errno));

	va_start(args, format);
	sb_vefunc("eerror", COLOR_RED, format, args);
	va_end(args);

	sb_dump_backtrace();

	sb_maybe_gdb();

	abort();
}
