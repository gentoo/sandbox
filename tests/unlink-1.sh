#!/bin/sh
# make sure we catch openat with relative path
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

export SANDBOX_LOG=$PWD/sb.log
unlink-0 -1 f
test -e sb.log
