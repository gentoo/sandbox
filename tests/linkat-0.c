#define CONFIG HAVE_LINKAT
#define FUNC linkat
#define SFUNC "linkat"
#define FUNC_STR "%i, \"%s\", %i, \"%s\", %x"
#define FUNC_IMP olddirfd, oldpath, newdirfd, newpath, flags
#define ARG_CNT 5
#define ARG_USE "<olddirfd> <oldpath> <newdirfd> <newpath> <flags>"

#define process_args() \
	s = argv[i++]; \
	int olddirfd = atoi(s); \
	\
	s = argv[i++]; \
	char *oldpath = s; \
	\
	s = argv[i++]; \
	int newdirfd = atoi(s); \
	\
	s = argv[i++]; \
	char *newpath = s; \
	\
	s = argv[i++]; \
	int flags = 0; \
	sscanf(s, "%i", &flags);

#include "test-skel-0.c"
