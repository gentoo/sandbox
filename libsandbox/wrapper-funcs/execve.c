/*
 * execve() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *path, char *const argv[], char *const envp[]
#define WRAPPER_ARGS path, argv, envp
#define EXEC_ARGS path, argv, my_env
#define EXEC_MY_ENV
#define EXEC_NO_PATH
#include "__wrapper_exec.c"
