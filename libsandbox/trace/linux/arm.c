#define trace_reg_ret ARM_r0

static int trace_get_sysnum(void *vregs)
{
	trace_regs *regs = vregs;
	return regs->ARM_r7;
}
static void trace_set_sysnum(void *vregs, long nr)
{
	trace_regs *regs = vregs;
	regs->ARM_r0 = nr;
	trace_set_regs(regs);
}

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	if (num < 7)
		return regs->uregs[num - 1];
	else
		return -1;
}
