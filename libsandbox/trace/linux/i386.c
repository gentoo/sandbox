#define trace_reg_sysnum orig_eax
#define trace_reg_ret eax

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
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
