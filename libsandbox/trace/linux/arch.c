#include "common.c"

/* Linux uses ptrace() */
#if !defined(HAVE_PTRACE) || !defined(HAVE_SYS_PTRACE_H) || !defined(HAVE_SYS_USER_H)
# define SB_NO_TRACE_ARCH
#elif defined(__bfin__)
# include "bfin.c"
#elif defined(__hppa__)
# include "hppa.c"
#elif defined(__i386__)
# include "i386.c"
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
# ifdef trace_sysnum_puser

static int trace_sysnum(void)
{
	return do_peekuser(trace_sysnum_puser);
}

static void trace_set_sysnum(void *vregs, long nr)
{
	do_pokeuser(trace_sysnum_puser, nr);
}

# endif
#endif
