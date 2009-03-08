#define CONFIG HAVE_SYMLINKAT
#define FUNC symlinkat
#define SFUNC "symlinkat"
#define FUNC_STR "\"%s\", %i, \"%s\""
#define FUNC_IMP oldpath, newdirfd, newpath
#define ARG_CNT 3
#define ARG_USE "<oldpath> <newdirfd> <newpath>"

#define process_args() \
	s = argv[i++]; \
	char *oldpath = s; \
	\
	s = argv[i++]; \
	int newdirfd = at_get_fd(s); \
	\
	s = argv[i++]; \
	char *newpath = s;

#include "test-skel-0.c"
