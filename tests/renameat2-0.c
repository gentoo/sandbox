#define CONFIG HAVE_RENAMEAT2
#define FUNC renameat2
#define SFUNC "renameat2"
#define FUNC_STR "%i, \"%s\", %i, \"%s\", %i"
#define FUNC_IMP olddirfd, oldpath, newdirfd, newpath, 0
#define ARG_CNT 4
#define ARG_USE "<dirfd>(old) <path>(old) <dirfd>(new) <path>(new)"

#define process_args() \
	s = argv[i++]; \
	int olddirfd = at_get_fd(s); \
	\
	s = argv[i++]; \
	char *oldpath = s; \
	\
	s = argv[i++]; \
	int newdirfd = at_get_fd(s); \
	\
	s = argv[i++]; \
	char *newpath = s;

#include "test-skel-0.c"
