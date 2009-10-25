/* Indexes into the pt_regs.u_reg[] array -- UREG_XX from kernel are all off
 * by 1 and use Ix instead of Ox.  These work for both 32 and 64 bit Linux.
 */
#define U_REG_G1 0
#define U_REG_O0 7

static int trace_sysnum_regs(void *vregs)
{
	trace_regs *regs = vregs;
	do_ptrace(PTRACE_GETREGS, regs, NULL);
	return regs->u_regs[U_REG_G1] ? : SB_SYS_EXECVE;
}
#define trace_sysnum_regs(regs) trace_sysnum_regs(regs)
#define trace_get_regs(regs) (0)

static long trace_raw_ret(void *vregs)
{
	trace_regs *regs = vregs;
	return regs->u_regs[U_REG_O0];
}

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	if (num < 7)
		return regs->u_regs[U_REG_O0 + num - 1];
	else
		return -1;
}
