#define FUNC removexattr
#define SFUNC "removexattr"
#define FUNC_STR "\"%s\", \"%s\""
#define FUNC_IMP path, name
#define ARG_CNT 2
#define ARG_USE "<path> <name>"

#define process_args() \
	s = argv[i++]; \
	char *path = s; \
	\
	s = argv[i++]; \
	char *name = s;

#include "test-skel-0.c"
