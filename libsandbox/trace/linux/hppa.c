static int trace_sysnum(void)
{
	return do_peekuser(20 * 4); /* PT_GR20 */
}

static long trace_raw_ret(void *vregs)
{
	trace_regs *regs = vregs;
	return regs->gr[28];
}

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

static long trace_get_regs(void *vregs)
{
	trace_regs *regs = vregs;
	size_t i;
	for (i = 21; i < 29; ++i)
		regs->gr[i] = do_peekuser(i * 4);
	return 0;
}
#define trace_get_regs trace_get_regs
