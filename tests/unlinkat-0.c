#define CONFIG HAVE_UNLINKAT
#define FUNC unlinkat
#define SFUNC "unlinkat"
#define FUNC_STR "%i, \"%s\", %x"
#define FUNC_IMP dirfd, path, flags
#define ARG_CNT 3
#define ARG_USE "<dirfd> <path> <flags>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = at_get_fd(s); \
	\
	s = argv[i++]; \
	char *path = s; \
	\
	s = argv[i++]; \
	int flags = at_get_flags(s);

#include "test-skel-0.c"
