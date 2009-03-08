#include "tests.h"

static int usage(int status)
{
	fputs(
		"Usage: get-group [args]\n"
		" `get-group`         return current gid\n"
		" `get-group file`    return gid of file\n"
		" `get-group -group`  return gid of group name\n"
		, status ? stderr : stdout
	);
	return status;
}

int main(int argc, char *argv[])
{
	switch (argc) {
		case 1:
			printf("%i\n", getgid());
			return 0;

		case 2:
			if (!strcmp(argv[1], "-h"))
				return usage(0);

			if (*argv[1] == '-') {
				const char *name = argv[1] + 1;
				struct group *grp = getgrnam(name);
				if (!grp)
					errp("getgrnam(%s) failed", name);
				printf("%i\n", grp->gr_gid);
			} else {
				const char *file = argv[1];
				struct stat st;
				if (lstat(file, &st))
					errp("lstat(%s) failed", file);
				printf("%i\n", st.st_gid);
			}
			return 0;
	}

	return usage(1);
}
