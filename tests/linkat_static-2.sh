#!/bin/sh
# basic functionality check (violations are caught)
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

addwrite $PWD
touch file || exit 1

adddeny $PWD
export SANDBOX_LOG=$PWD/sb.log
# we cant check exit status as ptrace code kills child
linkat-0 -1,EACCES AT_FDCWD file AT_FDCWD file-linked 0 #|| exit 1
test -s sb.log
