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
#define WRAPPER_SAFE() _SB_SAFE(SB_NR_UTIME, STRING_NAME, filename)
#include "utime.c"
