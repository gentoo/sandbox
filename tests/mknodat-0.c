/* strace -e raw=mknodat mknod ... */

#define CONFIG HAVE_MKNODAT
#define FUNC mknodat
#define SFUNC "mknodat"
#define FUNC_STR "%i, \"%s\", %o, %llx"
#define FUNC_IMP dirfd, file, mode, (unsigned long long)dev
#define ARG_CNT 4
#define ARG_USE "<dirfd> <file> <mode> <dev>"

#define process_args() \
	s = argv[i++]; \
	int dirfd = at_get_fd(s); \
	\
	s = argv[i++]; \
	const char *file = f_get_file(s); \
	\
	s = argv[i++]; \
	mode_t mode = sscanf_mode_t(s); \
	\
	s = argv[i++]; \
	dev_t dev = sscanf_dev_t(s);

#include "test-skel-0.c"
