#define CONFIG HAVE_OPEN64
#define FUNC open64
#define SFUNC "open64"
#define FUNC_STR "\"%s\", %o"
#define FUNC_IMP file, flags
#define ARG_CNT 2
#define ARG_USE "<file> <fflags>"

#define process_args() \
	s = argv[i++]; \
	const char *file = f_get_file(s); \
	\
	s = argv[i++]; \
	int flags = f_get_flags(s);

#include "test-skel-0.c"
