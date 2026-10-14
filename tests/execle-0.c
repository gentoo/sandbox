#define FUNC execle_test
#define SFUNC "execle"
#define FUNC_STR "\"%s\""
#define FUNC_IMP path
#define ARG_CNT 1
#define ARG_USE "<file>"
#define process_args() \
	s = argv[i++]; \
	const char *path = f_get_file(s);

static int execle_test(const char *path);

#include "test-skel-0.c"

static int execle_test(const char *path)
#define EXEC_FUNC execle(path, path, NULL, environ)
#include "exec-test-body.c"
#undef EXEC_FUNC
