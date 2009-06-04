/* strace -e raw=mknod mknod ... */

#define FUNC mknod
#define SFUNC "mknod"
#define FUNC_STR "\"%s\", %o, %llx"
#define FUNC_IMP file, mode, (unsigned long long)dev
#define ARG_CNT 3
#define ARG_USE "<file> <mode> <dev>"

#define process_args() \
	s = argv[i++]; \
	const char *file = f_get_file(s); \
	\
	s = argv[i++]; \
	mode_t mode = sscanf_mode_t(s); \
	\
	s = argv[i++]; \
	dev_t dev = sscanf_dev_t(s);

#include "test-skel-0.c"
