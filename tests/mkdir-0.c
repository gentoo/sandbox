#define FUNC mkdir
#define SFUNC "mkdir"
#define FUNC_STR "\"%s\", %o"
#define FUNC_IMP path, mode
#define ARG_CNT 2
#define ARG_USE "<path> <mode>"

#define process_args() \
	s = argv[i++]; \
	char *path = s; \
	\
	s = argv[i++]; \
	mode_t mode = sscanf_mode_t(s);

#include "test-skel-0.c"
