#!/bin/sh
# basic functionality check

addwrite $PWD

mkstemps-0 -1 "" 0 || exit 1
mkstemps-0 -1 ff 0 || exit 1
exec mkstemps-0 3 $PWD/f.XXXXXX 0
