#undef _trace_possible
#define _trace_possible _trace_possible

#ifdef SB_PERSONALITIES

static const struct syscall_entry syscall_table_32[] = {
#ifdef SB_PERSONALITIES_s390
#define S(s) { SB_SYS_s390_##s, SB_NR_##s, #s },
#include "trace_syscalls_s390.h"
#undef S
#endif
	{ SB_NR_UNDEF, SB_NR_UNDEF, NULL },
};
static const struct syscall_entry syscall_table_64[] = {
#ifdef SB_PERSONALITIES_s390x
#define S(s) { SB_SYS_s390x_##s, SB_NR_##s, #s },
#include "trace_syscalls_s390x.h"
#undef S
#endif
	{ SB_NR_UNDEF, SB_NR_UNDEF, NULL },
};

static bool pers_is_31(trace_regs *regs)
{
	return regs->psw.mask & 0x100000000ul ? false : true;
}

static const struct syscall_entry *trace_check_personality(void *vregs)
{
	trace_regs *regs = vregs;
	return pers_is_31(regs) ? syscall_table_32 : syscall_table_64;
}

static bool _trace_possible(const void *data)
{
	return true;
}

#else

static bool _trace_possible(const void *data)
{
# ifdef __s390x__
#  define ELFCLASS ELFCLASS64
# else
#  define ELFCLASS ELFCLASS32
# endif
	const Elf64_Ehdr *ehdr = data;
	return (ehdr->e_ident[EI_CLASS] == ELFCLASS) &&
		(ehdr->e_machine == EM_S390);
}

#endif

#undef trace_get_regs
static long trace_get_regs(void *vregs)
{
	ptrace_area parea = {
		/* Most regs we want are at the start (pswmask, pswaddr, and gpr0...gpr7,
		 * but the orig_gpr2 is much further along in the area. */
		.len = PT_ORIGGPR2 + sizeof(long),
		.kernel_addr = PT_PSWMASK,
		.process_addr = (uintptr_t)vregs,
	};
	do_ptrace(PTRACE_PEEKUSR_AREA, &parea, NULL);
	return 0;
}

#undef trace_set_regs
static long trace_set_regs(void *vregs)
{
	ptrace_area parea = {
		/* Most regs we want are at the start (pswmask, pswaddr, and gpr0...gpr7,
		 * but the orig_gpr2 is much further along in the area. */
		.len = PT_ORIGGPR2 + sizeof(long),
		.kernel_addr = PT_PSWMASK,
		.process_addr = (uintptr_t)vregs,
	};
	do_ptrace(PTRACE_POKEUSR_AREA, &parea, NULL);
	return 0;
}

#define trace_reg_sysnum gprs[2]
#define trace_reg_ret gprs[2]

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	if (num == 1)
		return regs->orig_gpr2;
	else if (num < 7)
		return regs->gprs[2 + num - 1];
	else
		return -1;
}
