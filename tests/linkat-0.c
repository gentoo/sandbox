#define CONFIG HAVE_LINKAT
#define FUNC linkat
#define SFUNC "linkat"
#define FUNC_STR "%i, \"%s\", %i, \"%s\", %x"
#define FUNC_IMP olddirfd, oldpath, newdirfd, newpath, flags
#define ARG_CNT 5
#define ARG_USE "<olddirfd> <oldpath> <newdirfd> <newpath> <flags>"

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
	char *newpath = s; \
	\
	s = argv[i++]; \
	int flags = at_get_flags(s);

#include "test-skel-0.c"
