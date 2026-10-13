/*
 * execvp() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *path, char *const argv[]
#define WRAPPER_ARGS path, argv
#define EXEC_ARG_FILE path
#define EXEC_USE_PATH
#include "__wrapper_exec.c"
