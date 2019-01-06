/*
 * A simple wrapper for execv that validates environment
 * is not corrupted by wrapper: https://bugs.gentoo.org/669702
 */

#define _GNU_SOURCE /* for environ */
#include <stdio.h>
#include "tests.h"

int main(int argc, char *argv[])
{
	char* execv_argv[] = {"nope", (char*)NULL,};
	char* execv_environ[] = {"FOO=1", (char*)NULL,};
	environ = execv_environ;
	execv("./does/not/exist", execv_argv);
	if (environ != execv_environ) {
		fprintf(stderr, "environ was changed unexpectedly by execv wrapper\n");
		return 1;
	}
	return 0;
}
