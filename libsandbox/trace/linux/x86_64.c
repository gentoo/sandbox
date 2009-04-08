#ifdef SB_SCHIZO

static const struct syscall_entry syscall_table_32[] = {
#define S(s) { SB_SYS_x86_##s, SB_NR_##s, #s },
#include "trace_syscalls_x86.h"
#undef S
	{ SB_NR_UNDEF, SB_NR_UNDEF, NULL },
};
static const struct syscall_entry syscall_table_64[] = {
#define S(s) { SB_SYS_x86_64_##s, SB_NR_##s, #s },
#include "trace_syscalls_x86_64.h"
#undef S
	{ SB_NR_UNDEF, SB_NR_UNDEF, NULL },
};

static bool pers_is_32(void)
{
	switch (do_peekuser(8 * CS)) {
		case 0x23: return true;
		case 0x33: return false;
		default:   sb_abort();
	}
}

static const struct syscall_entry *trace_check_personality(void)
{
	return pers_is_32() ? syscall_table_32 : syscall_table_64;
}

#else

static bool pers_is_32(void)
{
	return false;
}

#endif

static int trace_sysnum(void)
{
	return do_peekuser(8 * ORIG_RAX);
}

static long trace_raw_ret(void *vregs)
{
	trace_regs *regs = vregs;
	return pers_is_32() ? (int)regs->rax : regs->rax;
}

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	if (pers_is_32())
		switch (num) {
			case 1: return regs->rbx;
			case 2: return regs->rcx;
			case 3: return regs->rdx;
			case 4: return regs->rsi;
			case 5: return regs->rdi;
			case 6: return regs->rbp;
			default: return -1;
		}
	else
		switch (num) {
			case 1: return regs->rdi;
			case 2: return regs->rsi;
			case 3: return regs->rdx;
			case 4: return regs->r10;
			case 5: return regs->r8;
			case 6: return regs->r9;
			default: return -1;
		}
}

#ifdef DEBUG
static void trace_dump_regs(void *vregs)
{
	trace_regs *regs = vregs;
	sb_printf("{ ");
#define D(r) sb_printf(#r":%lu ", regs->r)
	D(rax);
	D(rdi);
	D(rsi);
	D(rdx);
	D(r10);
	D(r8);
	D(r9);
#undef D
	sb_printf("}");
}
#endif
