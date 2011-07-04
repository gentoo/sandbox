#define CONFIG HAVE_MKSTEMP
#define FUNC mkstemp
#define SFUNC "mkstemp"
#define FUNC_STR "\"%s\""
#define FUNC_IMP template
#define ARG_CNT 1
#define ARG_USE "<template>"

#define process_args() \
	s = argv[i++]; \
	char *template = s;

#include "test-skel-0.c"
