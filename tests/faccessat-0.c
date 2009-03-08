#define CONFIG HAVE_FACCESSAT
#define FUNC faccessat
#define SFUNC "faccessat"
#define FUNC_STR "%i, \"%s\", %x, %x"
#define FUNC_IMP dirfd, file, mode, flags
#define ARG_CNT 4
#define ARG_USE "<dirfd> <file> <mode> <flags>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = at_get_fd(s); \
	\
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	int mode = 0; \
	sscanf(s, "%i", &mode); \
	\
	s = argv[i++]; \
	int flags = at_get_flags(s);

#include "test-skel-0.c"
