#define FUNC fopen
#define SFUNC "fopen"
#define FUNC_STR "\"%s\", \"%s\""
#define FUNC_IMP file, mode
#define ARG_CNT 2
#define ARG_USE "<file> <mode>"

#define process_args() \
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	char *mode = s;

#include "test-skel-0.c"
