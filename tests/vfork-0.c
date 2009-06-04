#include "tests.h"

int main(int argc, char *argv[])
{
	int ret = 0;
	size_t i;

	if (argc == 1) {
		printf("Usage: %s <prog> [prog...]\n", argv[0]);
		return 1;
	}

	for (i = 1; i < argc; ++i) {
		size_t new_cnt = 0;
		char **new_argv = NULL;
		char *tok = strtok(argv[i], " ");
		while (tok) {
			++new_cnt;
			new_argv = realloc(new_argv, sizeof(*new_argv) * (new_cnt + 1));
			new_argv[new_cnt - 1] = tok;
			tok = strtok(NULL, " ");
		}
		new_argv[new_cnt] = NULL;

		switch (vfork()) {
			case -1:
				errp("vfork() failed");

			case 0:
				_exit(execvp(new_argv[0], new_argv));

			default: {
				int status;
				if (wait(&status) == -1)
					ret |= 1;
				else if (!WIFEXITED(status) || WEXITSTATUS(status))
					ret |= 1;
			}
		}
	}

	return 0;
}
