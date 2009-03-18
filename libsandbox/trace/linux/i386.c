static int trace_sysnum(void)
{
	return do_peekuser(4 * ORIG_EAX);
}

static long trace_raw_ret(void *vregs)
{
	struct user_regs_struct *regs = vregs;
	return regs->eax;
}

static unsigned long trace_arg(void *vregs, int num)
{
	struct user_regs_struct *regs = vregs;
	switch (num) {
		case 1: return regs->ebx;
		case 2: return regs->ecx;
		case 3: return regs->edx;
		case 4: return regs->esi;
		case 5: return regs->edi;
		case 6: return regs->ebp;
		default: return -1;
	}
}
