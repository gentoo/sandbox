#define CONFIG HAVE_MKDIRAT
#define FUNC mkdirat
#define SFUNC "mkdirat"
#define FUNC_STR "%i, \"%s\", %o"
#define FUNC_IMP dirfd, path, mode
#define ARG_CNT 2
#define ARG_USE "<dirfd> <path> <mode>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = atoi(s); \
	\
	s = argv[i++]; \
	char *path = s; \
	\
	s = argv[i++]; \
	mode_t mode; \
	sscanf(s, "%i", &mode);

#include "test-skel-0.c"
