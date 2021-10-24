#undef trace_regs
#define trace_regs struct ptrace_syscall_info

#define trace_reg_sysnum entry.nr
#define trace_reg_ret exit.rval

#undef trace_get_regs
#define trace_get_regs(regs) do_ptrace(PTRACE_GET_SYSCALL_INFO, (void *)(uintptr_t)sizeof(trace_regs), regs)

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	if (num < 7)
		return regs->entry.args[num - 1];
	else
		return -1;
}

#undef trace_set_regs
static long trace_set_regs(void *vregs)
{
	sb_ewarn("sandbox: Unable to block violation\n");
	return 0;
}
