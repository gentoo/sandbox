#define CONFIG HAVE_OPENAT
#define FUNC openat
#define SFUNC "openat"
#define FUNC_STR "%i, \"%s\", %o"
#define FUNC_IMP dirfd, file, flags
#define ARG_CNT 3
#define ARG_USE "<dirfd> <file> <flags>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = atoi(s); \
	\
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	int flags; \
	sscanf(s, "%i", &flags);

#include "test-skel-0.c"
