#include "common.c"

#undef SB_NO_TRACE
#if 0
#elif defined(__linux__)
# include "linux/arch.c"
#else
# define SB_NO_TRACE_OS
#endif

#ifdef SB_NO_TRACE_OS
# warning "trace: sorry, no support for your OS"
# define SB_NO_TRACE
#endif
