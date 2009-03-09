#define FUNC mkfifo
#define SFUNC "mkfifo"
#define FUNC_STR "\"%s\", %o"
#define FUNC_IMP file, mode
#define ARG_CNT 2
#define ARG_USE "<file> <mode>"

#define process_args() \
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	mode_t mode = sscanf_mode_t(s);

#include "test-skel-0.c"
