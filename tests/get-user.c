#include "tests.h"

static int usage(int status)
{
	fputs(
		"Usage: get-user [args]\n"
		" `get-user`         return current uid\n"
		" `get-user file`    return uid of file\n"
		" `get-user -user`   return uid of user name\n"
		, status ? stderr : stdout
	);
	return status;
}

int main(int argc, char *argv[])
{
	switch (argc) {
		case 1:
			printf("%i\n", getuid());
			return 0;

		case 2:
			if (!strcmp(argv[1], "-h"))
				return usage(0);

			if (*argv[1] == '-') {
				const char *name = argv[1] + 1;
				struct passwd *pwd = getpwnam(name);
				if (!pwd)
					errp("getpwnam(%s) failed", name);
				printf("%i\n", pwd->pw_uid);
			} else {
				const char *file = argv[1];
				struct stat st;
				if (lstat(file, &st))
					errp("lstat(%s) failed", file);
				printf("%i\n", st.st_uid);
			}
			return 0;
	}

	return usage(1);
}
