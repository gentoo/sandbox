#!/bin/sh
# make sure tracing doesnt swallow valid SIGCHLD events
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

sigsuspend-zsh_tst
d=$?
echo "ret = $d"

sigsuspend-zsh_static_tst
s=$?
echo "ret = $s"

exit $(( d | s ))
