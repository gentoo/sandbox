#!/bin/sh
# basic open tests with static binaries
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

addwrite $PWD
open_static-0 3 ok "O_WRONLY|O_CREAT" 0666 || exit 1
open_static-0 3 ok O_RDONLY 0666 || exit 1

export SANDBOX_LOG=$PWD/sb.log
mkdir deny || exit 1
adddeny $PWD/deny
open_static-0 -1 deny/not-ok "O_WRONLY|O_CREAT" 0666
test -e sb.log
