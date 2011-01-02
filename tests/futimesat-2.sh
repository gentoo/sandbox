#!/bin/sh
# basic functionality check (violations are caught)

adddeny $PWD
export SANDBOX_LOG=$PWD/sb.log
futimesat-0 -1,EACCES AT_FDCWD . NULL || exit 1
test -s sb.log
