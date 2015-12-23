/*
 * execve() wrapper.
 *
 * Copyright 1999-2015 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int dirfd, const char *path, char *const argv[], char *const envp[], int flags
#define WRAPPER_ARGS dirfd, path, argv, envp, flags
#define EXEC_ARGS dirfd, path, argv, my_env, flags
#define EXEC_MY_ENV
#define EXEC_NO_PATH
#include "__wrapper_exec.c"
