#define trace_reg_sysnum orig_p0
#define trace_reg_ret r0

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	switch (num) {
		case 1: return regs->r0;
		case 2: return regs->r1;
		case 3: return regs->r2;
		case 4: return regs->r3;
		case 5: return regs->r4;
		case 6: return regs->r5;
		default: return -1;
	}
}
