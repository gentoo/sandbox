#define REG_R0 0
#define REG_A0 16

#define trace_reg_sysnum r0

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	if (num < 7)
		return *(&regs->r16 + num - 1);
	else
		return -1;
}

static long do_peekuser(long offset)
{
	return do_ptrace(PTRACE_PEEKUSER, (void *)offset, NULL);
}

static long do_pokeuser(long offset, long val)
{
	return do_ptrace(PTRACE_POKEUSER, (void *)offset, (void *)val);
}

#undef trace_get_regs
static long trace_get_regs(void *vregs)
{
	trace_regs *regs = vregs;
	size_t i;
	regs->r0 = do_peekuser(REG_R0);
	for (i = 0; i < 7; ++i)
		*(&regs->r16 + i) = do_peekuser(REG_A0 + i);
	return 0;
}

#undef trace_set_regs
static long trace_set_regs(void *vregs)
{
	trace_regs *regs = vregs;
	size_t i;
	do_pokeuser(REG_R0, regs->r0);
	for (i = 0; i < 7; ++i)
		do_pokeuser(REG_A0 + i, *(&regs->r16 + i));
	return 0;
}

static long trace_raw_ret(void *vregs)
{
	trace_regs *regs = vregs;
	return regs->r16;
}

static void trace_set_ret(void *vregs, int err)
{
	/* We set r19 here, but it looks like trace_set_regs does not sync that.
	 * Remember that {r16..r31} == {a0..a15}, so when we write out {a0..a5},
	 * we also write out {r16..r21} -- a3 == r19.
	 */
	trace_regs *regs = vregs;
	regs->r0 = err;
	regs->r19 = -1;
	trace_set_regs(regs);
}
