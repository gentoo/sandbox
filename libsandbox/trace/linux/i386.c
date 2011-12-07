#undef _trace_possible
#define _trace_possible _trace_possible
static bool _trace_possible(const void *data)
{
	/* i386 can only trace i386 :( */
	const Elf64_Ehdr *ehdr = data;
	return (ehdr->e_ident[EI_CLASS] == ELFCLASS32) &&
		(ehdr->e_machine == EM_386);
}

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
