#define WRAPPER_ARGS_PROTO pid_t *pid, \
	const char *path, \
	const posix_spawn_file_actions_t *file_actions, \
	const posix_spawnattr_t *attrp, \
	char *const argv[], \
	char *const envp[]
#define WRAPPER_ARGS pid, path, file_actions, attrp, argv, envp
#define EXEC_ARGS pid, path, file_actions, attrp, argv, my_env
#define EXEC_ARG_FILE path
#define EXEC_MY_ENV
#define EXEC_RETURN_ERRNO
#include "__wrapper_exec.c"
