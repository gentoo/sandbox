#define CONFIG HAVE_MKSTEMP64
#define FUNC mkstemp64
#define SFUNC "mkstemp64"
#define FUNC_STR "\"%s\""
#define FUNC_IMP template
#define ARG_CNT 1
#define ARG_USE "<template>"

#define process_args() \
	s = argv[i++]; \
	char *template = s;

#define _LARGEFILE64_SOURCE
#include "test-skel-0.c"
