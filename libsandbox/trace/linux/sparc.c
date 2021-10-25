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
