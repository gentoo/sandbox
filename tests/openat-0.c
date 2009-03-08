#define CONFIG HAVE_OPENAT
#define FUNC openat
#define SFUNC "openat"
#define FUNC_STR "%i, \"%s\", %o"
#define FUNC_IMP dirfd, file, flags
#define ARG_CNT 3
#define ARG_USE "<dirfd> <file> <flags>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = at_get_fd(s); \
	\
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	int flags = at_get_flags(s);

#include "test-skel-0.c"
