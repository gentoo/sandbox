#define FUNC open
#define SFUNC "open"
#define FUNC_STR "\"%s\", 0x%x, %o"
#define FUNC_IMP file, flags, mode
#define ARG_CNT 3
#define ARG_USE "<file> <flags> <mode>"

#define process_args() \
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	int flags = f_get_flags(s); \
	\
	s = argv[i++]; \
	mode_t mode = sscanf_mode_t(s);

#include "test-skel-0.c"
