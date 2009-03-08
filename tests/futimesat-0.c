#define CONFIG HAVE_FUTIMESAT
#define FUNC futimesat
#define SFUNC "futimesat"
#define FUNC_STR "%i, \"%s\", %p"
#define FUNC_IMP dirfd, file, times
#define ARG_CNT 3
#define ARG_USE "<dirfd> <file> <times:=NULL>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = at_get_fd(s); \
	\
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	const struct timeval *times = NULL;

#include "test-skel-0.c"
