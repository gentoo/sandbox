#undef trace_regs
#define trace_regs struct ptrace_syscall_info

#define trace_reg_sysnum entry.nr

#undef trace_get_regs
#define trace_get_regs(regs) do_ptrace(PTRACE_GET_SYSCALL_INFO, (void *)(uintptr_t)sizeof(trace_regs), regs)

#ifndef PTRACE_SET_SYSCALL_INFO
#define PTRACE_SET_SYSCALL_INFO 0x4212
#endif

#undef trace_set_regs
#define trace_set_regs(regs) do_ptrace(PTRACE_SET_SYSCALL_INFO, (void *)(uintptr_t)sizeof(trace_regs), regs)

static long trace_raw_ret(void *vregs)
{
	trace_regs *regs = vregs;
	return regs->exit.rval;
}

static void trace_set_ret(void *vregs, int err)
{
	trace_regs *regs = vregs;
	regs->exit.rval = -err;
	regs->exit.is_error = 1;
	trace_set_regs(regs);
}

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	if (num < 7)
		return regs->entry.args[num - 1];
	else
		return -1;
}
