#include "tests.h"

#ifndef CONFIG
# define CONFIG 1
#endif

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

	test_ret = 0;
	i = 1;
	while (i < argc) {
		char *s;

		s = argv[i++];
		long ret = atoi(s);

		process_args();

		long actual_ret = (long)FUNC(FUNC_IMP);
		printf("%s: " SFUNC "(" FUNC_STR ") = %li (wanted %li)\n",
			(actual_ret == ret) ? "PASS" : "FAIL",
			FUNC_IMP, actual_ret, ret);
		if (actual_ret != ret) ++test_ret;
	}

	return test_ret;
#else
	puts("not implemented");
	return 77;
#endif
}
