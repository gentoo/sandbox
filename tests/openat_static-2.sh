#!/bin/sh
# make sure we catch openat with absolute path
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

addwrite $PWD
openat_static-0 3 AT_FDCWD ok "O_WRONLY|O_CREAT" 0666 || exit 1
openat_static-0 3 AT_FDCWD ok O_RDONLY 0666 || exit 1

export SANDBOX_LOG=$PWD/sb.log
mkdir deny || exit 1
adddeny $PWD/deny
openat_static-0 -1 AT_FDCWD $PWD/deny/not-ok "O_WRONLY|O_CREAT" 0666
test -e sb.log
