#define CONFIG HAVE_UTIMENSAT
#define FUNC utimensat
#define SFUNC "utimensat"
#define FUNC_STR "%i, \"%s\", %p, %x"
#define FUNC_IMP dirfd, file, times, flags
#define ARG_CNT 4
#define ARG_USE "<dirfd> <file> <times> <atflags>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = at_get_fd(s); \
	\
	s = argv[i++]; \
	const char *file = f_get_file(s); \
	\
	s = argv[i++]; \
	const struct timespec *times = NULL; \
	\
	s = argv[i++]; \
	int flags = at_get_flags(s);

#include "test-skel-0.c"
