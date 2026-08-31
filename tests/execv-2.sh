#!/bin/sh
# a traced exec that fails must not free the caller's environment

addwrite $PWD

# set*id makes the exec wrapper hand this to the tracer rather than run it
# in-process, and dropping +x makes the exec itself fail, so the wrapper
# returns to its caller instead of being replaced.  Only exercises the
# traced path when the suite is not run as root.
cp "$(command -v sb_true)" noexec || exit 1
chmod u+s,a-x noexec || exit 1

./noexec
test $? -eq 126
