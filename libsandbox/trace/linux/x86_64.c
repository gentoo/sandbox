static int trace_sysnum(void)
{
	return do_peekuser(8 * ORIG_RAX);
}

static long trace_raw_ret(void *vregs)
{
	struct user_regs_struct *regs = vregs;
	return regs->rax;
}

static unsigned long trace_arg(void *vregs, int num)
{
	struct user_regs_struct *regs = vregs;
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
