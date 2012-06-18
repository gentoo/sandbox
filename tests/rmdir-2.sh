#!/bin/sh
# make sure symlinks don't fail

addwrite $PWD

mkdir reject || exit 1
adddeny $PWD/reject

ln -s reject foo || exit 1
rmdir-0 -1,ENOTDIR foo || exit 1
[ -e foo ] || exit 1
! test -e sandbox.log
