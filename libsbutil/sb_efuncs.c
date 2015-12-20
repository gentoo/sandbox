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

/*
 * First try to write to the known good message log location (which is
 * normally tied to the initial sandbox's stderr).  If that fails, fall
 * back to writing to /dev/tty.  While this might annoy some people,
 * using stderr will break tests that try to validate output. #261957
 * Other related bugs on the topic: #278761
 */
static void sb_vefunc(const char *prog, const char *color, const char *format, va_list args)
{
	bool opened;
	int fd;

	if (likely(sbio_message_path))
		fd = sbio_open(sbio_message_path, O_WRONLY|O_APPEND|O_CLOEXEC, 0);
	else
		fd = -1;
	if (fd == -1)
		fd = sbio_open(sbio_fallback_path, O_WRONLY|O_CLOEXEC, 0);
	opened = (fd != -1);
	if (fd == -1)
		fd = fileno(stderr);

	if (color)
		sb_fdprintf(fd, " %s*%s ", color, COLOR_NORMAL);
	sb_vfdprintf(fd, format, args);

	if (opened)
		close(fd);
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

/* This is a bit of a hack to expose the same file logic to generic printers.
 * Probably want to revisit sb_vefunc and move the guts there to a new func.
 */
void sb_eraw(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	sb_vefunc(NULL, NULL, format, args);
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
