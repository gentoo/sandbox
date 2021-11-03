#!/bin/sh
# make sure tracing doesnt swallow valid SIGCHLD events
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

sigsuspend-zsh_tst
d=$?
echo "ret = $d"

# We can't trace static children currently with YAMA ptrace_scope 1+.
[ ${at_yama_ptrace_scope} -gt 0 ] && exit ${d}

sigsuspend-zsh_static_tst
s=$?
echo "ret = $s"

exit $(( d | s ))
