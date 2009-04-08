static int trace_sysnum(void)
{
	return do_peekuser(PT_ORIG_P0);
}

static long trace_raw_ret(void *vregs)
{
	trace_regs *regs = vregs;
	return regs->r0;
}

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
