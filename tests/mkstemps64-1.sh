#!/bin/sh
# basic functionality check

addwrite $PWD

mkstemps64-0 -1 "" 0 || exit 1
mkstemps64-0 -1 ff 0 || exit 1
exec mkstemps64-0 3 $PWD/f.XXXXXX 0
