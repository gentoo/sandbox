#!/bin/sh
# basic functionality check (violations are caught)

adddeny $PWD
export SANDBOX_LOG=$PWD/sb.log
utimensat-0 -1,EACCES AT_FDCWD . NULL 0 || exit 1
test -s sb.log
