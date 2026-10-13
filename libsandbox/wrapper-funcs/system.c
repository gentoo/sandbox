/*
 * system() wrapper.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Licensed under the GPL-2
 */

#define WRAPPER_ARGS_PROTO const char *command
#define WRAPPER_ARGS command
#include "__wrapper_exec.c"
