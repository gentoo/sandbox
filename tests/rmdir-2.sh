#!/bin/sh
# make sure symlinks don't fail

export SANDBOX_LOG=$PWD/sb.log

addwrite $PWD

mkdir reject || exit 1
adddeny $PWD/reject

ln -s reject foo || exit 1
rmdir-0 -1,ENOTDIR foo || exit 1
[ -e foo ] || exit 1
! test -e sb.log
