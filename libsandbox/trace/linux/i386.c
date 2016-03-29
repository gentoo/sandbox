#undef _trace_possible
#define _trace_possible _trace_possible
static bool _trace_possible(const void *data)
{
	/* i386 can only trace i386 :( */
	const Elf64_Ehdr *ehdr = data;
	return (ehdr->e_ident[EI_CLASS] == ELFCLASS32) &&
		(ehdr->e_machine == EM_386);
}

#ifdef SB_SCHIZO
static const struct syscall_entry syscall_table[] = {
#define S(s) { SB_SYS_x86_##s, SB_NR_##s, #s },
#include "trace_syscalls_x86.h"
#undef S
	{ SB_NR_UNDEF, SB_NR_UNDEF, NULL },
};
# define trace_check_personality(regs) syscall_table
#endif

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
