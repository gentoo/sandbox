/*
 * sb_backtrace.c
 *
 * Need to keep in a dedicated file so libsandbox can override.
 *
 * Copyright 1999-2012 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

__attribute__((weak))
void __sb_dump_backtrace(void)
{
}
