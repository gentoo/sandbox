/*
 * popen() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *command, const char *type
#define WRAPPER_ARGS command, type
#define WRAPPER_RET_TYPE FILE *
#define WRAPPER_RET_DEFAULT NULL
#define EXEC_NO_FILE
#include "__wrapper_exec.c"
