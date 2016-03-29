#!/bin/sh
# basic functionality check

addwrite $PWD

mkostemp64-0 -1 "" 0 || exit 1
mkostemp64-0 -1 ff 0 || exit 1
exec mkostemp64-0 3 $PWD/f.XXXXXX 0
