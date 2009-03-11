/*
 * execvp() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *path, char *const argv[]
#define WRAPPER_ARGS path, argv
#include "__wrapper_exec.c"
