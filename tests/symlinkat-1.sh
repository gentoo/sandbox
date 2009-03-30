#!/bin/sh
# basic functionality check

addwrite $PWD

touch file
symlinkat-0 0 file AT_FDCWD sym || exit 1
[ -e file -a -L sym ]
