#!/bin/sh
# basic functionality check

addwrite $PWD

mkostemps64-0 -1 "" 0 0 || exit 1
mkostemps64-0 -1 ff 0 0 || exit 1
exec mkostemps64-0 3 $PWD/f.XXXXXX 0 0
