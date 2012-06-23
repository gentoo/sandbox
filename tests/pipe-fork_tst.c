/*
https://bugs.gentoo.org/364877
written by Victor Stinner <victor.stinner@haypocalc.com>
*/

#include "headers.h"

static void cloexec(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFD);
	if (flags == -1) {
		perror("fcntl(F_GETFD)");
		exit(1);
	}
	fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
}

int main(int argc, char *argv[])
{
	int err;
	pid_t child;
	int outpipe[2];
	int errpipe[2];
	ssize_t n;
	char buffer[4096];
	char *dir = dirname(argv[0]);
	char *argv0 = "pipe-fork_static_tst";
	char *child_argv[] = {argv0, NULL};

	if (dir)
		if (chdir(dir)) {}

	err = pipe(outpipe);
	if (err) {
		perror("open");
		return 1;
	}

	err = pipe(errpipe);
	if (err) {
		perror("open");
		return 1;
	}
	cloexec(errpipe[0]);
	cloexec(errpipe[1]);

	child = fork();
	if (child < 0) {
		perror("fork");
		return 1;
	}
	if (child == 0) {
		close(outpipe[0]);
		close(errpipe[0]);

		dup2(outpipe[1], 1);
		execvp(argv0, child_argv);
		execv(argv0, child_argv);

		perror("execvp");
		return 1;
	} else {
		close(outpipe[1]);
		close(errpipe[1]);

		printf("wait errpipe..."); fflush(stdout);
		n = read(errpipe[0], buffer, sizeof(buffer));
		if (n < 0) {
			perror("fcntl(F_GETFD)");
			return 1;
		}
		printf(" done ");

		while (1) {
			n = read(outpipe[0], buffer, sizeof(buffer));
			if (n < 0) {
				perror("fcntl(F_GETFD)");
				return 1;
			}
			if (n == 0)
				break;
		}

		int status;
		waitpid(child, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			printf("OK!");
			return 0;
		}
		printf("\nfailed! child status: %#x\n", status);
		return 1;
	}
}
