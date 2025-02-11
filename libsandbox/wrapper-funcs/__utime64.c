/*
 * __utime64() wrapper.
 *
 * Copyright 1999-2021 Gentoo Foundation
 * Licensed under the GPL-2
 */

/*
 * NB: Reusing the 32-bit time interface isn't entirely correct as the 64-bit time interface uses a
 * different structure, but we never decode the time values in sandbox, so it doesn't matter to use.
 */
#undef WRAPPER_NR
#define WRAPPER_NR SB_NR_UTIME
#include "utime.c"
