#!/bin/sh
# make sure we catch openat with relative path
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

addwrite $PWD
openat_static-0 3 AT_FDCWD ok "O_WRONLY|O_CREAT" 0666 || exit 1
openat_static-0 3 AT_FDCWD ok O_RDONLY 0666 || exit 1

mkdir deny || exit 1
adddeny $PWD/deny
openat_static-0 -1,EPERM AT_FDCWD deny/not-ok "O_WRONLY|O_CREAT" 0666 || exit 1
test -e sandbox.log
