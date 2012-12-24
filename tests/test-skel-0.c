#include "tests.h"

const char *color_normal = "\033[0m";
const char *color_green  = "\033[32;01m";
const char *color_yellow = "\033[33;01m";
const char *color_red    = "\033[31;01m";

#ifndef CONFIG
# define CONFIG 1
#endif

#define V_TIMESPEC "NULL"
#define V_STRMODE "<r|w|a>[+bcemx] (see `man 3 fopen`)"

static bool _strtoul(const char *sul, unsigned long *ul)
{
	char *e;
	*ul = strtoul(sul, &e, 0);
	return (*e == '\0');
}

static int _get_flags(const char *str_flags, const value_pair flags[])
{
	const char *delim = "|";
	char *tok = strtok(strdup(str_flags), delim);
	int ret = 0;
	while (tok) {
		bool found;
		ret |= _lookup_val(flags, tok, &found);
		if (!found) {
			int a;
			sscanf(tok, "%i", &a);
			ret |= a;
		}
		tok = strtok(NULL, delim);
	}
	return ret;
}

#define V_FFLAGS "O_XXX flags (see `man 2 open`)"
int f_get_flags(const char *str_flags)
{
	const value_pair flags[] = {
		PAIR(O_APPEND)
		PAIR(O_CREAT)
		PAIR(O_DIRECTORY)
		PAIR(O_EXCL)
		PAIR(O_NOCTTY)
		PAIR(O_NOFOLLOW)
		PAIR(O_RDONLY)
		PAIR(O_RDWR)
		PAIR(O_TRUNC)
		PAIR(O_WRONLY)
		{ }
	};
	if (!str_flags)
		return O_CREAT | O_RDWR;
	return _get_flags(str_flags, flags);
}

#define V_FILE "NULL | path"
const char *f_get_file(const char *str_file)
{
	if (!strcmp(str_file, "NULL"))
		return NULL;
	else
		return str_file;
}

#define V_ATFLAGS "0 | AT_SYMLINK_NOFOLLOW | AT_REMOVEDIR | AT_SYMLINK_FOLLOW | AT_EACCESS"
int at_get_flags(const char *str_flags)
{
	const value_pair flags[] = {
		PAIR(AT_SYMLINK_NOFOLLOW)
		PAIR(AT_REMOVEDIR)
		PAIR(AT_SYMLINK_FOLLOW)
		PAIR(AT_EACCESS)
		{ }
	};
	return _get_flags(str_flags, flags);
}

#define V_MODE_T "0x# (for hex) | 0# (for octal) | # (for decimal)"
mode_t sscanf_mode_t(const char *str_mode)
{
	/* some systems (like Linux) have a 32bit mode_t.  Others
	 * (like FreeBSD) have a 16bit mode_t.  We can't straight
	 * sscanf() into it otherwise we might smash the stack.
	 */
	int mode;
	/* Default to full access.  */
	if (!str_mode)
		return 0777;
	sscanf(str_mode, "%i", &mode);
	return (mode_t)mode;
}

#define V_DEV_T V_MODE_T
dev_t sscanf_dev_t(const char *str_dev)
{
	/* Similar issue with dev_t as mode_t.  Can't assume that
	 * sizeof(dev_t) == sizeof(int).  Often dev_t is 64bit.
	 */
	int dev;
	sscanf(str_dev, "%i", &dev);
	return (dev_t)dev;
}

#define V_DIRFD "AT_FDCWD | fd # | path[:<flags>[:<mode>]]"
int at_get_fd(const char *str_dirfd)
{
	/* work some magic ... expected format:
	 *	- AT_FDCWD
	 *	- number
	 *	- path[:<flags>[:<mode>]]
	 */
	if (!strcmp(str_dirfd, "AT_FDCWD"))
		return AT_FDCWD;

	char *str = strdup(str_dirfd);
	char *str_path, *str_flags, *str_mode;

	str_path = strtok(str, ":");
	str_flags = strtok(NULL, ":");
	if (str_flags == NULL) {
		unsigned long dirfd;
		if (_strtoul(str_dirfd, &dirfd))
			return dirfd;
	}
	str_mode = strtok(NULL, ":");

	return open(str_path, f_get_flags(str_flags), sscanf_mode_t(str_mode));
}

#define V_ACCESS_MODE "r | w | x | f"
int access_mode(const char *s)
{
	int ret = 0;
	if (strchr(s, 'r')) ret |= R_OK;
	if (strchr(s, 'w')) ret |= W_OK;
	if (strchr(s, 'x')) ret |= X_OK;
	if (strchr(s, 'f')) ret  = F_OK;
	return ret;
}

int main(int argc, char *argv[])
{
#if CONFIG
	int i, test_ret;

	if ((argc - 1) % (ARG_CNT + 1) || argc == 1) {
#define _ARG_USE "<ret> " ARG_USE
		printf(
			"usage: " SFUNC " <tests>\n"
			"test: < " _ARG_USE " >\n"
			"\n"
		);
		const char *vusage[] = {
			"<ret>", "#[,<errno>]; # is a decimal and errno can be symbolic",
			"<dirfd>", V_DIRFD,
			"<file>", V_FILE,
			"<times>", V_TIMESPEC,
			"<atflags>", V_ATFLAGS,
			"<fflags>", V_FFLAGS,
			"<mode>", V_MODE_T,
			"<strmode>", V_STRMODE,
			"<acc_mode>", V_ACCESS_MODE,
			"<dev>", V_DEV_T,
			"<uid>", "# (decimal)",
			"<gid>", "# (decimal)",
		};
		for (i = 0; i < ARRAY_SIZE(vusage); i += 2)
			if (strstr(_ARG_USE, vusage[i]))
				printf("%-10s := %s\n", vusage[i], vusage[i + 1]);
		exit(1);
	}

	if (getenv("NOCOLOR"))
		color_normal = color_green = color_yellow = color_red = "";

	test_ret = 0;
	i = 1;
	while (i < argc) {
		char *s;

		s = argv[i++];
		long want_ret = atoi(s);

		int want_errno = 0;
		s = strchr(s, ',');
		if (s++) {
			want_errno = lookup_errno(s);
			if (!want_errno)
				err("unable to lookup errno '%s'", s);
		}

		process_args();

		errno = 0;
		long actual_ret = (long)FUNC(FUNC_IMP);
		int actual_errno = errno;

		bool passed_ret = want_ret == actual_ret;
		bool passed_errno = !want_errno || want_errno == actual_errno;
		bool passed = passed_ret && passed_errno;
#define COLOR(b) (b ? color_green : color_red)

		printf(
			"%s%s%s: " SFUNC "(" FUNC_STR ") = "
			"%s%li (wanted %li)%s; "
			"%serrno = %s:%i [%s] (wanted %s:%i [%s])%s\n",
			COLOR(passed), passed ? "PASS" : "FAIL", color_normal,
#if ARG_CNT
			FUNC_IMP,
#endif
			(passed ? "" : COLOR(passed_ret)),
			actual_ret, want_ret, color_normal,
			(!passed && want_errno) ? COLOR(passed_errno) : "",
			rev_lookup_errno(actual_errno), actual_errno, strerror(actual_errno),
			rev_lookup_errno(want_errno), want_errno, strerror(want_errno),
			color_normal);

		if (!passed) ++test_ret;
	}

	return test_ret;
#else
	puts("not implemented");
	return 77;
#endif
}
