#!/bin/sh
# basic functionality check

addwrite $PWD

touch old
renameat-0 0 AT_FDCWD old AT_FDCWD new || exit 1
[ ! -e old -a -e new ]
