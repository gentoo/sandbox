#define CONFIG HAVE_MKSTEMPS64
#define FUNC mkstemps64
#define SFUNC "mkstemps64"
#define FUNC_STR "\"%s\", %i"
#define FUNC_IMP template, suffixlen
#define ARG_CNT 2
#define ARG_USE "<template> <suffixlen>"

#define process_args() \
	s = argv[i++]; \
	char *template = s; \
	\
	s = argv[i++]; \
	int suffixlen = 0; \
	sscanf(s, "%i", &suffixlen);

#include "test-skel-0.c"
