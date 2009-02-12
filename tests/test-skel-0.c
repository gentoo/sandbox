#include "tests.h"

#ifndef CONFIG
# define CONFIG 1
#endif

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
		int ret = atoi(s);

		process_args();

		int actual_ret = (int)FUNC(FUNC_IMP);
		printf("%s: " SFUNC "(" FUNC_STR ") = %i (wanted %i)\n",
			(actual_ret == ret) ? "PASS" : "FAIL",
			FUNC_IMP, actual_ret, ret);
		if (actual_ret != ret) ++test_ret;
	}

	return test_ret;
#else
	puts("not implemented");
	return 0;
#endif
}
