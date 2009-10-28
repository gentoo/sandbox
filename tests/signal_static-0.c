/* trace code needs to handle random child signals */
#include "tests.h"

int main(int argc, char *argv[])
{
	if (argc == 1)
		/* don't dump usage since testsuite will try to exec us
		 * for possible skip status
		 */
		return 0;

	char pre = argv[1][0];
	int sig;
	switch (pre) {
		case '+':
		case '-':
			++argv[1];
		default:
			sig = lookup_signal(argv[1]);
	}

	int ret;
	switch (pre) {
		case '+': ret = (signal(sig, SIG_DFL) == SIG_ERR); break;
		case '-': ret = (signal(sig, SIG_IGN) == SIG_ERR); break;
		default:  ret = kill(getpid(), sig);  break;
	}

	if (argc > 2) {
		argv += 2;
		return execv(argv[0], argv);
	} else
		return ret;
}
