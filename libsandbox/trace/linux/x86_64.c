#undef _trace_possible
#define _trace_possible _trace_possible

#ifdef SB_SCHIZO

static const struct syscall_entry syscall_table_32[] = {
#ifdef SB_SCHIZO_x86
#define S(s) { SB_SYS_x86_##s, SB_NR_##s, #s },
#include "trace_syscalls_x86.h"
#undef S
#endif
	{ SB_NR_UNDEF, SB_NR_UNDEF, NULL },
};
static const struct syscall_entry syscall_table_64[] = {
#ifdef SB_SCHIZO_x86_64
#define S(s) { SB_SYS_x86_64_##s, SB_NR_##s, #s },
#include "trace_syscalls_x86_64.h"
#undef S
#endif
	{ SB_NR_UNDEF, SB_NR_UNDEF, NULL },
};
static const struct syscall_entry syscall_table_x32[] = {
#ifdef SB_SCHIZO_x32
#define S(s) { SB_SYS_x32_##s, SB_NR_##s, #s },
#include "trace_syscalls_x32.h"
#undef S
#endif
	{ SB_NR_UNDEF, SB_NR_UNDEF, NULL },
};

static bool pers_is_32(trace_regs *regs)
{
	switch (regs->cs) {
		case 0x23: return true;
		case 0x33: return false;
		default:   sb_ebort("unknown x86_64 (CS) personality");
	}
}

static bool pers_is_x32(trace_regs *regs)
{
	switch (regs->ds) {
		case 0x2b: return true;
		case 0x00: return false;
		default:   sb_ebort("unknown x86_64 (DS) personality");
	}
}

static const struct syscall_entry *trace_check_personality(void *vregs)
{
	trace_regs *regs = vregs;
	if (pers_is_32(regs))
		return syscall_table_32;
	else if (pers_is_x32(regs))
		return syscall_table_x32;
	else
		return syscall_table_64;
}

static bool _trace_possible(const void *data)
{
	/* x86_64 can trace anything, but x32 can't trace x86_64 */
#if defined(__x86_64__) && defined(__ILP32__)
	const Elf64_Ehdr *ehdr = data;
	return (ehdr->e_ident[EI_CLASS] == ELFCLASS32);
#else
	return true;
#endif
}

#else

static bool pers_is_32(trace_regs *regs)
{
	return false;
}

static bool _trace_possible(const void *data)
{
	const Elf64_Ehdr *ehdr = data;
	return (ehdr->e_ident[EI_CLASS] == ELFCLASS64) &&
		(ehdr->e_machine == EM_X86_64);
}

#endif

#define trace_reg_sysnum orig_rax

static long trace_raw_ret(void *vregs)
{
	trace_regs *regs = vregs;
	return pers_is_32(regs) ? (int)regs->rax : regs->rax;
}

static void trace_set_ret(void *vregs, int err)
{
	trace_regs *regs = vregs;
	regs->rax = -err;
	trace_set_regs(regs);
}

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	if (pers_is_32(regs))
		/* 32bit x86 */
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
#define D(r) sb_printf(#r":%"PRIu64" ", regs->r)
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
