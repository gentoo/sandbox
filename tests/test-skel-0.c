#include "tests.h"

const char *color_normal = "\033[0m";
const char *color_green  = "\033[32;01m";
const char *color_yellow = "\033[33;01m";
const char *color_red    = "\033[31;01m";

#ifndef CONFIG
# define CONFIG 1
#endif

#define PAIR(x) { #x, x },

int at_get_fd(const char *str_dirfd)
{
	if (!strcmp(str_dirfd, "AT_FDCWD"))
		return AT_FDCWD;
	else
		return atoi(str_dirfd);
}

int at_get_flags(const char *str_flags)
{
	if (!strcmp(str_flags, "AT_SYMLINK_NOFOLLOW"))
		return AT_SYMLINK_NOFOLLOW;
	else {
		int flags = 0;
		sscanf(str_flags, "%i", &flags);
		return flags;
	}
}

int f_get_flags(const char *str_flags)
{
	const char *delim = "|";
	char *tok = strtok(strdup(str_flags), delim);
	struct {
		const char *name;
		int val;
	} const flags[] = {
		PAIR(O_APPEND)
		PAIR(O_CREAT)
		PAIR(O_EXCL)
		PAIR(O_NOCTTY)
		PAIR(O_NOFOLLOW)
		PAIR(O_RDONLY)
		PAIR(O_RDWR)
		PAIR(O_TRUNC)
		PAIR(O_WRONLY)
	};
	int i, ret = 0;
	while (tok) {
		for (i = 0; i < ARRAY_SIZE(flags); ++i)
			if (!strcmp(tok, flags[i].name)) {
				ret |= flags[i].val;
				break;
			}
		if (i == ARRAY_SIZE(flags)) {
			int a;
			sscanf(tok, "%i", &a);
			ret |= a;
		}
		tok = strtok(NULL, delim);
	}
	return ret;
}

mode_t sscanf_mode_t(const char *str_mode)
{
	/* some systems (like Linux) have a 32bit mode_t.  Others
	 * (like FreeBSD) have a 16bit mode_t.  We can't straight
	 * sscanf() into it otherwise we might smash the stack.
	 */
	int mode;
	sscanf(str_mode, "%i", &mode);
	return (mode_t)mode;
}

int lookup_errno(const char *str_errno)
{
	struct {
		const char *name;
		int val;
	} const tbl[] = {
		PAIR(EINVAL)
		PAIR(ENOENT)
		PAIR(EPERM)
	};
	int i;
	for (i = 0; i < ARRAY_SIZE(tbl); ++i)
		if (!strcmp(str_errno, tbl[i].name))
			return tbl[i].val;
	return 0;
}

int main(int argc, char *argv[])
{
#if CONFIG
	int i, test_ret;

	if ((argc - 1) % (ARG_CNT + 1) || argc == 1) {
		printf(
			"usage: " SFUNC " <tests>\n"
			"test: < <ret> " ARG_USE " >\n"
		);
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
			"%serrno = %i [%s] (wanted %i [%s])%s\n",
			COLOR(passed), passed ? "PASS" : "FAIL", color_normal,
			FUNC_IMP,
			(passed ? "" : COLOR(passed_ret)),
			actual_ret, want_ret, color_normal,
			(!passed && want_errno) ? COLOR(passed_errno) : "",
			actual_errno, strerror(actual_errno),
			want_errno, strerror(want_errno), color_normal);

		if (!passed) ++test_ret;
	}

	return test_ret;
#else
	puts("not implemented");
	return 77;
#endif
}
