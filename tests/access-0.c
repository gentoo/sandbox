#define FUNC access
#define SFUNC "access"
#define FUNC_STR "\"%s\", %x"
#define FUNC_IMP file, mode
#define ARG_CNT 2
#define ARG_USE "<file> <acc_mode>"

#define process_args() \
	s = argv[i++]; \
	const char *file = f_get_file(s); \
	\
	s = argv[i++]; \
	int mode = access_mode(s);

#include "test-skel-0.c"
