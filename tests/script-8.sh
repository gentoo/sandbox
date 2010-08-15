#!/bin/sh
# make sure tracing doesnt swallow valid SIGCHLD events
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

sigsuspend-zsh_tst || exit 1
sigsuspend-zsh_static_tst || exit 2
