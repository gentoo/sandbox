#!/bin/sh
# basic functionality check

addwrite $PWD

mkstemp-0 -1 "" || exit 1
mkstemp-0 -1 ff || exit 1
exec mkstemp-0 3 $PWD/f.XXXXXX
