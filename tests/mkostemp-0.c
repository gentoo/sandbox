#define CONFIG HAVE_MKOSTEMP
#define FUNC mkostemp
#define SFUNC "mkostemp"
#define FUNC_STR "\"%s\", %#x"
#define FUNC_IMP template, flags
#define ARG_CNT 2
#define ARG_USE "<template> <fflags>"

#define process_args() \
	s = argv[i++]; \
	char *template = s; \
	\
	s = argv[i++]; \
	int flags = f_get_flags(s);

#include "test-skel-0.c"
