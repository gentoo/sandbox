#!/bin/sh
# basic functionality check

addwrite $PWD

touch f
unlinkat-0 0 AT_FDCWD f 0 || exit 1

mkdir d
unlinkat-0 0 AT_FDCWD d AT_REMOVEDIR || exit 2
