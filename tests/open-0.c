#define FUNC open
#define SFUNC "open"
#define FUNC_STR "\"%s\", %o"
#define FUNC_IMP file, flags
#define ARG_CNT 2
#define ARG_USE "<file> <flags>"

#define process_args() \
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	int flags; \
	sscanf(s, "%i", &flags);

#include "test-skel-0.c"
