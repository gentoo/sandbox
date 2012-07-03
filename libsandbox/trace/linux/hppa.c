#define trace_reg_sysnum (20 * 4)	/* PT_GR20 */
#define trace_reg_ret (28 * 4)	/* PT_GR28 */

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	switch (num) {
		case 1: return regs->gr[26];
		case 2: return regs->gr[25];
		case 3: return regs->gr[24];
		case 4: return regs->gr[23];
		case 5: return regs->gr[22];
		case 6: return regs->gr[21];
		default: return -1;
	}
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
	for (i = 20; i < 29; ++i)
		regs->gr[i] = do_peekuser(i * 4);
	return 0;
}

#undef trace_set_regs
static long trace_set_regs(void *vregs)
{
	trace_regs *regs = vregs;
	size_t i;
	for (i = 20; i < 29; ++i)
		do_pokeuser(i * 4, regs->gr[i]);
	return 0;
}
