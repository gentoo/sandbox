#!/bin/sh
# basic functionality check

addwrite $PWD

touch file || exit 1
symlink-0 0 file sym || exit 1
[ -e file -a -L sym ]
