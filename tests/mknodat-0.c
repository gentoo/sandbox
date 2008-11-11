#define FUNC mknodat
#define SFUNC "mknodat"
#define FUNC_STR "%i, \"%s\", %o, %x"
#define FUNC_IMP dirfd, file, mode, dev
#define ARG_CNT 4
#define ARG_USE "<dirfd> <file> <mode> <dev>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = atoi(s); \
	\
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	mode_t mode; \
	sscanf(s, "%i", &mode); \
	\
	s = argv[i++]; \
	dev_t dev; \
	sscanf(s, "%i", &dev);

#include "test-skel-0.c"
