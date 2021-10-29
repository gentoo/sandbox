/*
 * Handle command line arguments
 *
 * Copyright 1999-2015 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"
#include "sandbox.h"

/* Setting to -1 will load defaults from the config file. */
int opt_use_namespaces = -1;
int opt_use_ns_cgroup = -1;
int opt_use_ns_ipc = -1;
int opt_use_ns_mnt = -1;
int opt_use_ns_net = -1;
int opt_use_ns_pid = -1;
int opt_use_ns_sysv = -1;
int opt_use_ns_time = -1;
int opt_use_ns_user = -1;
int opt_use_ns_uts = -1;
bool opt_use_bash = false;

static const struct {
	const char *name;
	int *opt;
	int default_val;
} config_opts[] = {
	/* Default these to off until they can get more testing. */
	{ "NAMESPACES_ENABLE",       &opt_use_namespaces, false, },
	{ "NAMESPACE_CGROUP_ENABLE", &opt_use_ns_cgroup,  false, },
	{ "NAMESPACE_IPC_ENABLE",    &opt_use_ns_ipc,     false, },
	{ "NAMESPACE_MNT_ENABLE",    &opt_use_ns_mnt,     false, },
	{ "NAMESPACE_NET_ENABLE",    &opt_use_ns_net,     false, },
	{ "NAMESPACE_PID_ENABLE",    &opt_use_ns_pid,     false, },
	{ "NAMESPACE_SYSV_ENABLE",   &opt_use_ns_sysv,    false, },
	{ "NAMESPACE_TIME_ENABLE",   &opt_use_ns_time,    false, },
	{ "NAMESPACE_USER_ENABLE",   &opt_use_ns_user,    false, },
	{ "NAMESPACE_UTS_ENABLE",    &opt_use_ns_uts,     false, },
};

static void read_config(void)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(config_opts); ++i) {
		int *opt = config_opts[i].opt;
		if (*opt == -1)
			*opt = sb_get_cnf_bool(config_opts[i].name, config_opts[i].default_val);
	}
}

static const char sb_sonfigure_opts[] = SANDBOX_CONFIGURE_OPTS;

static void show_version(void)
{
	printf(
		"Gentoo path sandbox\n"
		" version: " PACKAGE_VERSION "\n"
		" C lib:   " LIBC_VERSION " (" LIBC_PATH ")\n"
		" build:   " __DATE__ " " __TIME__ "\n"
		" contact: " PACKAGE_BUGREPORT " via https://bugs.gentoo.org/\n"
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
		"\nconfigured with these options:\n%s\n",
		sb_sonfigure_opts
	);
	exit(0);
}

#define PARSE_FLAGS "+chV"
#define a_argument required_argument
static struct option const long_opts[] = {
	{"ns-on",         no_argument, &opt_use_namespaces, true},
	{"ns-off",        no_argument, &opt_use_namespaces, false},
	{"ns-cgroup-on",  no_argument, &opt_use_ns_cgroup, true},
	{"ns-cgroup-off", no_argument, &opt_use_ns_cgroup, false},
	{"ns-ipc-on",     no_argument, &opt_use_ns_ipc, true},
	{"ns-ipc-off",    no_argument, &opt_use_ns_ipc, false},
	{"ns-mnt-on",     no_argument, &opt_use_ns_mnt, true},
	{"ns-mnt-off",    no_argument, &opt_use_ns_mnt, false},
	{"ns-net-on",     no_argument, &opt_use_ns_net, true},
	{"ns-net-off",    no_argument, &opt_use_ns_net, false},
	{"ns-pid-on",     no_argument, &opt_use_ns_pid, true},
	{"ns-pid-off",    no_argument, &opt_use_ns_pid, false},
	{"ns-sysv-on",    no_argument, &opt_use_ns_sysv, true},
	{"ns-sysv-off",   no_argument, &opt_use_ns_sysv, false},
	{"ns-time-on",    no_argument, &opt_use_ns_time, true},
	{"ns-time-off",   no_argument, &opt_use_ns_time, false},
	{"ns-user-on",    no_argument, &opt_use_ns_user, true},
	{"ns-user-off",   no_argument, &opt_use_ns_user, false},
	{"ns-uts-on",     no_argument, &opt_use_ns_uts, true},
	{"ns-uts-off",    no_argument, &opt_use_ns_uts, false},
	{"bash",          no_argument, NULL, 'c'},
	{"help",          no_argument, NULL, 'h'},
	{"version",       no_argument, NULL, 'V'},
	{"run-configure", no_argument, NULL, 0x800},
	{NULL,            no_argument, NULL, 0x0}
};
static const char * const opts_help[] = {
	"Enable  the use of namespaces",
	"Disable the use of namespaces",
	"Enable  the use of cgroup namespaces",
	"Disable the use of cgroup namespaces",
	"Enable  the use of IPC (and System V) namespaces",
	"Disable the use of IPC (and System V) namespaces",
	"Enable  the use of mount namespaces",
	"Disable the use of mount namespaces",
	"Enable  the use of network namespaces",
	"Disable the use of network namespaces",
	"Enable  the use of process (pid) namespaces",
	"Disable the use of process (pid) namespaces",
	"Enable  the use of System V namespaces",
	"Disable the use of System V namespaces",
	"Enable  the use of time namespaces",
	"Disable the use of time namespaces",
	"Enable  the use of user namespaces",
	"Disable the use of user namespaces",
	"Enable  the use of UTS (hostname/uname) namespaces",
	"Disable the use of UTS (hostname/uname) namespaces",
	"Run command through bash shell",
	"Print this help and exit",
	"Print version and exit",
	"Run local sandbox configure in same way and exit (developer only)",
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

	fprintf(fp, "\nContact: " PACKAGE_BUGREPORT " via https://bugs.gentoo.org/\n");

	exit(status);
}

static void run_configure(int argc, char *argv[])
{
	int i;
	char *cmd;
	xasprintf(&cmd, "set -x; ./configure %s", sb_sonfigure_opts);
	/* This doesn't need to be fast, so keep it simple. */
	for (i = optind; i < argc; ++i)
		xasprintf(&cmd, "%s %s", cmd, argv[i]);
	exit(system(cmd));
}

void parseargs(int argc, char *argv[])
{
	int i;

	while ((i = getopt_long(argc, argv, PARSE_FLAGS, long_opts, NULL)) != -1) {
		switch (i) {
		case 'c':
			opt_use_bash = true;
			break;
		case 'V':
			show_version();
		case 'h':
			show_usage(0);
		case 0x800:
			run_configure(argc, argv);
		case '?':
			show_usage(1);
		}
	}

	read_config();
}
