#!/bin/sh
# make sure we catch openat with relative path
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

touch f
test -e f || exit 1
adddeny "${PWD}"
unlink_static-0 -1,EPERM f || exit 1
test -e sandbox.log -a -e f
