#define FUNC fchownat
#define SFUNC "fchownat"
#define FUNC_STR "%i, \"%s\", %i, %i, %x"
#define FUNC_IMP dirfd, file, uid, gid, flags
#define ARG_CNT 5
#define ARG_USE "<dirfd> <file> <uid> <gid>, <flags>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = atoi(s); \
	\
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	uid_t uid = atoi(s); \
	\
	s = argv[i++]; \
	gid_t gid = atoi(s); \
	\
	s = argv[i++]; \
	int flags = 0; \
	sscanf(s, "%i", &flags);

#include "test-skel-0.c"
