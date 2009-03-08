#define CONFIG HAVE_FCHOWNAT
#define FUNC fchownat
#define SFUNC "fchownat"
#define FUNC_STR "%i, \"%s\", %i, %i, %x"
#define FUNC_IMP dirfd, file, uid, gid, flags
#define ARG_CNT 5
#define ARG_USE "<dirfd> <file> <uid> <gid> <flags>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = at_get_fd(s); \
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
	int flags = at_get_flags(s);

#include "test-skel-0.c"
