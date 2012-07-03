/*
 * Some arches (like hppa) don't implement PTRACE_GETREGS, while others (like
 * sparc) swap the meaning of "addr" and "data.  What a bunch of asshats.
 */
#define trace_get_regs(regs) do_ptrace(PTRACE_GETREGS, NULL, regs)
#define trace_set_regs(regs) do_ptrace(PTRACE_SETREGS, NULL, regs)

static int trace_errno(long err)
{
	return (err < 0 && err > -4096) ? err * -1 : 0;
}

static long trace_result(void *vregs, int *error)
{
	long sr = trace_raw_ret(vregs);
	*error = trace_errno(sr);
	return *error ? -1 : sr;
}
