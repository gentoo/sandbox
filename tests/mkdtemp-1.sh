#!/bin/sh
# basic functionality check

addwrite $PWD

mkdtemp-0 0 ""        || exit 1
mkdtemp-0 0 fffffffff || exit 1
# mkdtemp() returns a pointer, so any non-zero value is OK
mkdtemp-0 0 $PWD/f.XXXXXX && exit 1
exit 0
