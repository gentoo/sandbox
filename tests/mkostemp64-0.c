#define CONFIG HAVE_MKOSTEMP64
#define FUNC mkostemp64
#define SFUNC "mkostemp64"
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

#define _LARGEFILE64_SOURCE
#include "test-skel-0.c"
