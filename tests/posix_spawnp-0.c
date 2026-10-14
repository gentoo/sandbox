#define FUNC posix_spawnp
#define SFUNC "posix_spawnp"
#define FUNC_STR "%p, \"%s\", %p, %p, %p, %p"
#define FUNC_IMP NULL, file, NULL, NULL, av, environ
#define ARG_CNT 1
#define ARG_USE "<file>"
#define process_args() \
	s = argv[i++]; \
	const char *file = f_get_file(s); \
	char *av[] = { (char *)file, NULL };
#include "test-skel-0.c"
