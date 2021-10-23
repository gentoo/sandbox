/*
 * make sure some pre-checks are pulled in when needed
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#if SB_NR_IS_DEFINED(SB_NR_MKDIR) && !SB_NR_IS_DEFINED(SB_NR_MKDIRAT)
# include "mkdirat_pre_check.c"
#endif

#if SB_NR_IS_DEFINED(SB_NR_OPEN) && !SB_NR_IS_DEFINED(SB_NR_OPENAT)
# include "openat_pre_check.c"
#endif

#if SB_NR_IS_DEFINED(SB_NR_OPEN64) && !SB_NR_IS_DEFINED(SB_NR_OPENAT64)
# include "openat64_pre_check.c"
#endif

#if SB_NR_IS_DEFINED(SB_NR_UNLINK) && !SB_NR_IS_DEFINED(SB_NR_UNLINKAT)
# include "unlinkat_pre_check.c"
#endif

#include "__pre_at_check.c"
