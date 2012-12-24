#define CONFIG HAVE_OPENAT64
#define FUNC openat64
#define SFUNC "openat64"
#define FUNC_STR "%i, \"%s\", %#x, %o"
#define FUNC_IMP dirfd, file, flags, mode
#define ARG_CNT 4
#define ARG_USE "<dirfd> <file> <fflags> <mode>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = at_get_fd(s); \
	\
	s = argv[i++]; \
	const char *file = f_get_file(s); \
	\
	s = argv[i++]; \
	int flags = f_get_flags(s); \
	\
	s = argv[i++]; \
	mode_t mode = sscanf_mode_t(s);

#include "test-skel-0.c"
