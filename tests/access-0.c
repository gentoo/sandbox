#define FUNC access
#define SFUNC "access"
#define FUNC_STR "\"%s\", %x"
#define FUNC_IMP file, mode
#define ARG_CNT 2
#define ARG_USE "<file> <mode>"

#define process_args() \
	s = argv[i++]; \
	char *file = s; \
	\
	s = argv[i++]; \
	int mode = 0; \
	if (strchr(s, 'r')) mode |= R_OK; \
	if (strchr(s, 'w')) mode |= W_OK; \
	if (strchr(s, 'x')) mode |= X_OK; \
	if (strchr(s, 'f')) mode  = F_OK;

#include "test-skel-0.c"
