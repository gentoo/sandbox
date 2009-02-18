#define CONFIG HAVE_TRUNCATE64
#define FUNC truncate64
#define SFUNC "truncate64"
#define FUNC_STR "\"%s\", %i"
#define FUNC_IMP path, length
#define ARG_CNT 2
#define ARG_USE "<oldpath> <newpath>"

#define process_args() \
	s = argv[i++]; \
	char *path = s; \
	\
	s = argv[i++]; \
	off64_t length; \
	sscanf(s, "%i", &length);

#include "test-skel-0.c"
