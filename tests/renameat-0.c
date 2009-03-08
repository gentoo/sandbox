#define CONFIG HAVE_RENAMEAT
#define FUNC renameat
#define SFUNC "renameat"
#define FUNC_STR "%i, \"%s\", %i, \"%s\""
#define FUNC_IMP olddirfd, oldpath, newdirfd, newpath
#define ARG_CNT 4
#define ARG_USE "<olddirfd> <oldpath> <newdirfd> <newpath>"

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
