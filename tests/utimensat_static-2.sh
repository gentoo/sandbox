#!/bin/sh
# basic functionality check (violations are caught)
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

adddeny $PWD
export SANDBOX_LOG=$PWD/sb.log
# we cant check exit status as ptrace code kills child
utimensat_static-0 -1,EACCES AT_FDCWD . NULL 0 #|| exit 1
test -s sb.log
