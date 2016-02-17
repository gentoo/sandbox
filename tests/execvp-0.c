/*
 * A simple wrapper for execvp.  Useful when most host programs don't match
 * the ABI of the active libsandbox.so (e.g. 64bit vs 32bit).
 */

#include "tests.h"

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("usage: execvp <path> [argv0 [argvN] ...]\n");
		return 0;
	}
	return execvp(argv[1], argv + 2);
}
