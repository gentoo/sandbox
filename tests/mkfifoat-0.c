#define CONFIG HAVE_MKFIFOAT
#define FUNC mkfifoat
#define SFUNC "mkfifoat"
#define FUNC_STR "%i, \"%s\", %o"
#define FUNC_IMP dirfd, file, mode
#define ARG_CNT 3
#define ARG_USE "<dirfd> <file> <mode>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = atoi(s); \
	\
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	mode_t mode; \
	sscanf(s, "%i", &mode);

#include "test-skel-0.c"
