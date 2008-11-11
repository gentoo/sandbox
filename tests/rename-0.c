#define FUNC rename
#define SFUNC "rename"
#define FUNC_STR "\"%s\", \"%s\""
#define FUNC_IMP oldpath, newpath
#define ARG_CNT 2
#define ARG_USE "<oldpath> <newpath>"

#define process_args() \
	s = argv[i++]; \
	char *oldpath = s; \
	\
	s = argv[i++]; \
	char *newpath = s;

#include "test-skel-0.c"
