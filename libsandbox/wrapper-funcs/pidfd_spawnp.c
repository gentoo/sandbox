#define WRAPPER_ARGS_PROTO int *pidfd, \
	const char *file, \
	const posix_spawn_file_actions_t *file_actions, \
	const posix_spawnattr_t *attrp, \
	char *const argv[], \
	char *const envp[]
#define WRAPPER_ARGS pidfd, file, file_actions, attrp, argv, envp
#define EXEC_ARGS pidfd, file, file_actions, attrp, argv, my_env
#define EXEC_ARG_FILE file
#define EXEC_MY_ENV
#define EXEC_USE_PATH
#define EXEC_RETURN_ERRNO
#include "__wrapper_exec.c"
