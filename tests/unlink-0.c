#define FUNC unlink
#define SFUNC "unlink"
#define FUNC_STR "\"%s\""
#define FUNC_IMP path
#define ARG_CNT 1
#define ARG_USE "<path>"

#define process_args() \
	s = argv[i++]; \
	char *path = s;

#include "test-skel-0.c"
