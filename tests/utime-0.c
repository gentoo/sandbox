#define CONFIG HAVE_UTIME
#define FUNC utime
#define SFUNC "utime"
#define FUNC_STR "\"%s\", %p"
#define FUNC_IMP file, times
#define ARG_CNT 2
#define ARG_USE "<file> <times:=NULL>"

#define process_args() \
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	const struct utimbuf *times = NULL;

#include "test-skel-0.c"
