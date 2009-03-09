#define CONFIG HAVE_FCHMODAT
#define FUNC fchmodat
#define SFUNC "fchmodat"
#define FUNC_STR "%i, \"%s\", %o, %x"
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
	mode_t mode = sscanf_mode_t(s); \
	\
	s = argv[i++]; \
	int flags = at_get_flags(s);

#include "test-skel-0.c"
