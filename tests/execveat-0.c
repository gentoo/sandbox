#define CONFIG HAVE_EXECVEAT
#define FUNC execveat_test
#define SFUNC "execveat"
#define FUNC_STR "%d, \"%s\", %p, %p, %d"
#define FUNC_IMP dirfd, file, av, environ, flags
#define ARG_CNT 1
#define ARG_USE "<dirfd> <file> <atflags>"
#define process_args() \
	s = argv[i++]; \
	int dirfd = at_get_fd(s); \
	s = argv[i++]; \
	const char *file = f_get_file(s); \
	s = argv[i++]; \
	int flags = at_get_flags(s); \
	char *av[] = { (char *)file, NULL };

static int execveat_test(int dirfd, const char *path, char *const argv[], char *const envp[], int flags);

#include "test-skel-0.c"

#if CONFIG
static int execveat_test(int dirfd, const char *path, char *const argv[], char *const envp[], int flags)
#define EXEC_FUNC execveat(dirfd, path, argv, envp, flags)
#include "exec-test-body.c"
#undef EXEC_FUNC
#endif
