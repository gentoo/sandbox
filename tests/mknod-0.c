/* strace -e raw=mknod mknod ... */

#define FUNC mknod
#define SFUNC "mknod"
#define FUNC_STR "\"%s\", %o, %x"
#define FUNC_IMP file, mode, dev
#define ARG_CNT 3
#define ARG_USE "<file> <mode> <dev>"

#define process_args() \
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	mode_t mode = sscanf_mode_t(s); \
	\
	s = argv[i++]; \
	dev_t dev; \
	sscanf(s, "%i", &dev);

#include "test-skel-0.c"
