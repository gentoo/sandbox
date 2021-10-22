#define FUNC lsetxattr
#define SFUNC "lsetxattr"
#define FUNC_STR "\"%s\", \"%s\", \"%s\", %zu, %i"
#define FUNC_IMP path, name, value, size, flags
#define ARG_CNT 5
#define ARG_USE "<path> <name> <value> <size> <flags>"

#define process_args() \
	s = argv[i++]; \
	char *path = s; \
	\
	s = argv[i++]; \
	char *name = s; \
	\
	s = argv[i++]; \
	char *value = s; \
	\
	s = argv[i++]; \
	size_t size = atoi(s); \
	\
	s = argv[i++]; \
	int flags = atoi(s);

#include "test-skel-0.c"
