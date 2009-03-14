/*
 * Dump a QA warning
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "libsandbox.h"
#include "sbutil.h"
#include "wrappers.h"

/* First try to use the eqawarn program from portage.  If that fails, fall
 * back to writing to /dev/tty.  While this might annoy some people, using
 * stderr will break tests that try to validate output #261957.
 */
void sb_eqawarn(const char *format, ...)
{
	va_list args;
	FILE *fp;
	sighandler_t oldsig;
	bool is_pipe;

	/* If popen() fails, then writes to it will trigger SIGPIPE */
	oldsig = signal(SIGPIPE, SIG_IGN);

	fp = sb_unwrapped_popen("xargs eqawarn 2>/dev/null", "we");
	is_pipe = true;
	if (!fp) {
 do_tty:
		is_pipe = false;
		fp = fopen("/dev/tty", "ae");
		if (!fp)
			fp = stderr;
	}

	sb_fprintf(fp, "QA Notice: ");
	va_start(args, format);
	sb_vfprintf(fp, format, args);
	va_end(args);

	if (is_pipe) {
		int status = pclose(fp);
		if (WEXITSTATUS(status))
			goto do_tty;
	}

	signal(SIGPIPE, oldsig);
}
