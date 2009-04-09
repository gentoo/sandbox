/* trace code needs to handle random child signals */
#include "tests.h"

int main(int argc, char *argv[])
{
	if (argc == 1)
		/* don't dump usage since testsuite will try to exec us
		 * for possible skip status
		 */
		return 0;
	else
		return kill(getpid(), lookup_signal(argv[1]));
}
