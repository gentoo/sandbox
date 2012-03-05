#!/bin/sh
# make sure we catch openat with relative path
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

export SANDBOX_LOG=$PWD/sb.log
touch f
test -e f || exit 1
adddeny "${PWD}"
unlink_static-0 -1,EPERM f
test -e sb.log -a -e f
