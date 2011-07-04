#define CONFIG HAVE_MKSTEMPS
#define FUNC mkstemps
#define SFUNC "mkstemps"
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
