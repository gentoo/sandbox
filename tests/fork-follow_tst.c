/*
 * Make sure violations in children are caught.
 */

#include "tests.h"

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("usage: %s <number forks> <path to remove>\n", argv[0]);
		exit(1);
	}

	int i, forks = atoi(argv[1]);
	const char *path = argv[2];

	for (i = 0; i < forks; ++i) {
		pid_t pid = fork();
		if (pid < 0)
			errp("unable to fork");

		if (pid > 0) {
			/* parent -- wait for child */
			int status;
			if (waitpid(pid, &status, 0) == pid)
				exit(WEXITSTATUS(status));
			errp("waitpid failed");
		}
		/* child -- keep looping */
	}

	/* final child -- try to create the path */
	exit(creat(path, 0666) < 0 ? 0 : 1);
}
