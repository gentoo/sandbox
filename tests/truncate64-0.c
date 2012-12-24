#define CONFIG HAVE_TRUNCATE64
#define FUNC truncate64
#define SFUNC "truncate64"
#define FUNC_STR "\"%s\", %llu"
#define FUNC_IMP path, (unsigned long long)length
#define ARG_CNT 2
#define ARG_USE "<path> <length>"

#define process_args() \
	s = argv[i++]; \
	char *path = s; \
	\
	s = argv[i++]; \
	off64_t length; \
	unsigned long long sl; \
	sscanf(s, "%llu", &sl); \
	length = sl;

#include "test-skel-0.c"
