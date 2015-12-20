#include "common.c"

/* Linux uses ptrace().  We require PTRACE_SETOPTIONS so the exec tracing logic
 * is sane.  Otherwise we need a lot of arch-specific hacks to make it work.
 * This should be fine for linux-2.6+ versions.
 */
#if !defined(HAVE_PTRACE) || !defined(HAVE_SYS_PTRACE_H) || \
    !defined(HAVE_SYS_USER_H) || !defined(PTRACE_SETOPTIONS)
# define SB_NO_TRACE_ARCH
#elif defined(__alpha__)
# include "alpha.c"
#elif defined(__arm__)
# include "arm.c"
#elif defined(__bfin__)
# include "bfin.c"
#elif defined(__hppa__)
# include "hppa.c"
#elif defined(__i386__)
# include "i386.c"
#elif defined(__s390__)
# include "s390.c"
#elif defined(__sparc__)
# include "sparc.c"
#elif defined(__x86_64__)
# include "x86_64.c"
#else
# define SB_NO_TRACE_ARCH
#endif

#ifdef SB_NO_TRACE_ARCH
# warning "trace: sorry, no support for your architecture"
# define SB_NO_TRACE
#else

# ifdef trace_reg_sysnum
static int trace_get_sysnum(void *vregs)
{
	trace_regs *regs = vregs;
	return regs->trace_reg_sysnum;
}
static void trace_set_sysnum(void *vregs, long nr)
{
	trace_regs *regs = vregs;
	regs->trace_reg_sysnum = nr;
	trace_set_regs(regs);
}
# endif

# ifdef trace_reg_ret
static long trace_raw_ret(void *vregs)
{
	trace_regs *regs = vregs;
	return regs->trace_reg_ret;
}
static void trace_set_ret(void *vregs, int err)
{
	trace_regs *regs = vregs;
	regs->trace_reg_ret = -err;
	trace_set_regs(regs);
}
# endif

#endif
