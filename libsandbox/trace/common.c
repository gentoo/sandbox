struct syscall_entry {
	const int nr, sys;
	const char *name;
};

static int trace_get_sysnum(void *vregs);
static long trace_raw_ret(void *vregs);
static unsigned long trace_arg(void *vregs, int num);

#ifndef SB_PERSONALITIES
static const struct syscall_entry syscall_table[] = {
#define S(s) { SB_SYS_##s, SB_NR_##s, #s },
#include "trace_syscalls.h"
#undef S
	{ SB_NR_UNDEF, SB_NR_UNDEF, NULL },
};
# define trace_check_personality(regs) syscall_table
#endif
