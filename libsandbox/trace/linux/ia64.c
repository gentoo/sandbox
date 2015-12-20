#include <asm/ptrace_offsets.h>
#include <asm/rse.h>

/* We only care about two ptrace regs, so extract them ourselves rather than
 * get the "full" set via GETREGS.  We still need to extract the out regs by
 * hand either way.
 */
#undef trace_regs
struct sb_ia64_trace_regs {
	unsigned long r8, r10, r15;
	unsigned long out[6];
};
#define trace_regs struct sb_ia64_trace_regs

#define trace_reg_sysnum r15

static unsigned long trace_arg(void *vregs, int num)
{
	trace_regs *regs = vregs;
	if (num < 7)
		return regs->out[num - 1];
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
	unsigned long *out0, cfm, sof, sol;
	long rbs_end;

	regs->r15 = do_peekuser(PT_R15);

	/* Here there be gremlins! */
	rbs_end = do_peekuser(PT_AR_BSP);
	cfm = do_peekuser(PT_CFM);
	sof = (cfm >> 0) & 0x7f;
	sol = (cfm >> 7) & 0x7f;
	out0 = ia64_rse_skip_regs((unsigned long *)rbs_end, -sof + sol);
	for (i = 0; i < 7; ++i)
		regs->out[i] = do_peekdata((uintptr_t)ia64_rse_skip_regs(out0, i));

	return 0;
}

#undef trace_set_regs
static long trace_set_regs(void *vregs)
{
	trace_regs *regs = vregs;
	/* We only support rewriting of syscall/err # currently (not args). */
	do_pokeuser(PT_R8, regs->r8);
	do_pokeuser(PT_R10, regs->r10);
	do_pokeuser(PT_R15, regs->r15);
	return 0;
}

static long trace_raw_ret(void *vregs)
{
	trace_regs *regs = vregs;
	return regs->r8;
}

static void trace_set_ret(void *vregs, int err)
{
	trace_regs *regs = vregs;
	regs->r8 = err;
	regs->r10 = -1;
	trace_set_regs(regs);
}
