#define FUNC chown
#define SFUNC "chown"
#define FUNC_STR "\"%s\", %i, %i"
#define FUNC_IMP file, uid, gid
#define ARG_CNT 3
#define ARG_USE "<file> <uid> <gid>"

#define process_args() \
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	uid_t uid = atoi(s); \
	\
	s = argv[i++]; \
	gid_t gid = atoi(s);

#include "test-skel-0.c"
