#define trace_reg_ret regs[0]  /* x0 */

#undef trace_get_regs
static long trace_get_regs(void *vregs)
{
	struct iovec iov_regs = {
		.iov_base = vregs,
		.iov_len = sizeof(trace_regs),
	};
	return do_ptrace(PTRACE_GETREGSET, (void *)(uintptr_t)NT_PRSTATUS, &iov_regs);
}

#undef trace_set_regs
static long trace_set_regs(void *vregs)
{
	struct iovec iov_regs = {
		.iov_base = vregs,
		.iov_len = sizeof(trace_regs),
	};
	return do_ptrace(PTRACE_SETREGSET, (void *)(uintptr_t)NT_PRSTATUS, &iov_regs);
}

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	if (num < 7)
		return regs->regs[num - 1];  /* x0 - x5 */
	else
		return -1;
}

static int trace_get_sysnum(void *vregs)
{
	int nr;
	struct iovec iov_nr = {
		.iov_base = &nr,
		.iov_len = sizeof(nr),
	};
	do_ptrace(PTRACE_GETREGSET, (void *)(uintptr_t)NT_ARM_SYSTEM_CALL, &iov_nr);
	return nr;
}

static void trace_set_sysnum(void *vregs, int nr)
{
	struct iovec iov_nr = {
		.iov_base = &nr,
		.iov_len = sizeof(nr),
	};
	do_ptrace(PTRACE_SETREGSET, (void *)(uintptr_t)NT_ARM_SYSTEM_CALL, &iov_nr);
}
