/*
 * Handle command line arguments
 *
 * Copyright 1999-2015 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"
#include "sandbox.h"

static void show_version(void)
{
	puts(
		"Gentoo path sandbox\n"
		" version: " PACKAGE_VERSION "\n"
		" C lib:   " LIBC_VERSION " (" LIBC_PATH ")\n"
		" build:   " __DATE__ " " __TIME__ "\n"
		" contact: " PACKAGE_BUGREPORT " via http://bugs.gentoo.org/\n"
		" rtld:    "
#ifdef BROKEN_RTLD_NEXT
			"next is broken ;(\n"
#else
			"next is OK! :D\n"
#endif
#ifndef SB_SCHIZO
# define SB_SCHIZO "no"
#endif
		" schizo:  " SB_SCHIZO "\n"
		"\nconfigured with these options:\n"
		SANDBOX_CONFIGURE_OPTS
	);
	exit(0);
}

#define PARSE_FLAGS "+hV"
#define a_argument required_argument
static struct option const long_opts[] = {
	{"help",          no_argument, NULL, 'h'},
	{"version",       no_argument, NULL, 'V'},
	{NULL,            no_argument, NULL, 0x0}
};
static const char * const opts_help[] = {
	"Print this help and exit",
	"Print version and exit",
	NULL
};

static void show_usage(int status)
{
	const char a_arg[] = "<arg>";
	size_t a_arg_len = strlen(a_arg) + 2;
	size_t i;
	int optlen;
	FILE *fp = status ? stderr : stdout;

	fprintf(fp,
		"Usage: sandbox [options] [program [program args...]]\n"
		"\n"
		"Sandbox will start up a sandbox session and execute the specified program.\n"
		"If no program is specified, an interactive shell is automatically launched.\n"
		"You can use this to quickly test out sandbox behavior.\n"
		"\n"
		"Upon startup, initial settings are taken from these files / directories:\n"
		"\t" SANDBOX_CONF_FILE "\n"
		"\t" SANDBOX_CONFD_DIR "\n"
	);

	fprintf(fp, "\nOptions: -[%s]\n", PARSE_FLAGS + 1);
	/* prescan the --long opt length to auto-align */
	optlen = 0;
	for (i = 0; long_opts[i].name; ++i) {
		int l = strlen(long_opts[i].name);
		if (long_opts[i].has_arg == a_argument)
			l += a_arg_len;
		optlen = MAX(l, optlen);
	}

	for (i = 0; long_opts[i].name; ++i) {
		/* first output the short flag if it has one */
		if (long_opts[i].val > '~' || long_opts[i].val < ' ')
			printf("      ");
		else
			printf("  -%c, ", long_opts[i].val);

		/* then the long flag */
		if (long_opts[i].has_arg == no_argument)
			printf("--%-*s", optlen, long_opts[i].name);
		else
			printf("--%s %s %*s", long_opts[i].name, a_arg,
				(int)(optlen - strlen(long_opts[i].name) - a_arg_len), "");

		/* finally the help text */
		printf(" * %s\n", opts_help[i]);
	}

	fprintf(fp, "\nContact: " PACKAGE_BUGREPORT " via http://bugs.gentoo.org/\n");

	exit(status);
}

void parseargs(int argc, char *argv[])
{
	int i;

	while ((i = getopt_long(argc, argv, PARSE_FLAGS, long_opts, NULL)) != -1) {
		switch (i) {
		case 'V':
			show_version();
		case 'h':
			show_usage(0);
		case '?':
			show_usage(1);
		}
	}
}
