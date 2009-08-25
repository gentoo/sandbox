/*
 * make sure some pre-checks are pulled in when needed
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#if SB_NR_MKDIR != SB_NR_UNDEF && SB_NR_MKDIRAT == SB_NR_UNDEF
# include "mkdirat_pre_check.c"
#endif

#if SB_NR_OPEN != SB_NR_UNDEF && SB_NR_OPENAT == SB_NR_UNDEF
# include "openat_pre_check.c"
#endif

#if SB_NR_OPEN64 != SB_NR_UNDEF && SB_NR_OPENAT64 == SB_NR_UNDEF
# include "openat64_pre_check.c"
#endif

#if SB_NR_UNLINK != SB_NR_UNDEF && SB_NR_UNLINKAT == SB_NR_UNDEF
# include "unlinkat_pre_check.c"
#endif
