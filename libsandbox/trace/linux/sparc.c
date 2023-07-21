/* Since sparc's g0 register is hardcoded to 0 in the ISA, the kernel does not
 * bother copying it out when using the regs ptrace.  Instead it shifts things
 * by one and stores [g1..g7] in [0..6] and [o0..o7] in [7..14] (leaving the
 * last u_reg[15] unused).
 *
 * Oddly, the kernel defines are not adjusted to the values as they written in
 * the register structure -- UREG_G0 is 0, UREG_G1 is 1, etc...  So we have to
 * define our own to get correct behavior.  Also, the kernel uses UREG_I# when
 * it's easier for us to think of it in terms of UREG_O# (register windows!).
 *
 * This should work for both 32 and 64 bit Linux userland.
 */
#define U_REG_G1 0
#define U_REG_O0 7

#undef _trace_possible
#define _trace_possible _trace_possible

#ifdef SB_PERSONALITIES

static const struct syscall_entry syscall_table_32[] = {
#ifdef SB_PERSONALITIES_sparc
#define S(s) { SB_SYS_sparc_##s, SB_NR_##s, #s },
#include "trace_syscalls_sparc.h"
#undef S
#endif
	{ SB_NR_UNDEF, SB_NR_UNDEF, NULL },
};
static const struct syscall_entry syscall_table_64[] = {
#ifdef SB_PERSONALITIES_sparc64
#define S(s) { SB_SYS_sparc64_##s, SB_NR_##s, #s },
#include "trace_syscalls_sparc64.h"
#undef S
#endif
	{ SB_NR_UNDEF, SB_NR_UNDEF, NULL },
};

static bool pers_is_32(trace_regs *regs)
{
#ifdef __arch64__
	/* Sparc does not make it easy to detect 32-bit vs 64-bit.
	 * Inspect the syscall trap insn to see which one it is.
	 */
	unsigned long ret = do_ptrace(PTRACE_PEEKTEXT, (void *)regs->tpc, NULL);
	return (ret >> 32) == 0x91d02010;
#else
	return true;
#endif
}

static const struct syscall_entry *trace_check_personality(void *vregs)
{
	trace_regs *regs = vregs;
	if (pers_is_32(regs))
		return syscall_table_32;
	else
		return syscall_table_64;
}

static bool _trace_possible(const void *data)
{
#ifdef __arch64__
	/* sparc64 can trace sparc32. */
	return true;
#else
	/* sparc32 can only trace sparc32 :(. */
	const Elf64_Ehdr *ehdr = data;
	return ehdr->e_ident[EI_CLASS] == ELFCLASS32;
#endif
}

#else

static bool _trace_possible(const void *data)
{
	const Elf64_Ehdr *ehdr = data;
#ifdef __arch64__
	return ehdr->e_ident[EI_CLASS] == ELFCLASS64;
#else
	return ehdr->e_ident[EI_CLASS] == ELFCLASS32;
#endif
}

#endif

/* Sparc systems have swapped the addr/data args. */
#undef trace_get_regs
#undef trace_set_regs
#ifdef __arch64__
# define trace_get_regs(regs) do_ptrace(PTRACE_GETREGS64, regs, NULL)
# define trace_set_regs(regs) do_ptrace(PTRACE_SETREGS64, regs, NULL)
#else
# define trace_get_regs(regs) do_ptrace(PTRACE_GETREGS, regs, NULL)
# define trace_set_regs(regs) do_ptrace(PTRACE_SETREGS, regs, NULL)
#endif

#define trace_reg_sysnum u_regs[U_REG_G1]

static long trace_raw_ret(void *vregs)
{
	trace_regs *regs = vregs;
	return regs->u_regs[U_REG_O0];
}

static void trace_set_ret(void *vregs, int err)
{
	trace_regs *regs = vregs;
#ifndef __arch64__
	/* The carry bit is used to flag errors. */
	regs->psr |= PSR_C;
#else
	regs->tstate |= 0x1100000000;
#endif
	/* Userland negates the value on sparc. */
	regs->u_regs[U_REG_O0] = err;
	trace_set_regs(regs);
}

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	if (num < 7)
		return regs->u_regs[U_REG_O0 + num - 1];
	else
		return -1;
}
