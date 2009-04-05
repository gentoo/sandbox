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
		switch (vfork()) {
			case -1:
				errp("vfork() failed");

			case 0: {
				char *new_argv[] = { argv[i], NULL };
				_exit(execvp(argv[i], new_argv));
			}

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
