#!/bin/sh
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0
set -e
addwrite "$PWD/file"
faccessat_static-0 0 'file:O_RDWR|O_CREAT:0666' '' rw AT_EMPTY_PATH
exec 9<file
adddeny "$PWD/file"
faccessat_static-0 -1,EPERM 9 '' rw AT_EMPTY_PATH
