#!/bin/sh
# basic functionality check

addwrite $PWD

mkstemp64-0 -1 "" || exit 1
mkstemp64-0 -1 ff || exit 1
exec mkstemp64-0 3 $PWD/f.XXXXXX
