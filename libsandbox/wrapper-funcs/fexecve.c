/*
 * fexecve() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO int fd, char *const argv[], char *const envp[]
#define WRAPPER_ARGS fd, argv, envp
#define EXEC_ARGS fd, argv, my_env
#define EXEC_MY_ENV
#define EXEC_NO_FILE
#include "__wrapper_exec.c"
