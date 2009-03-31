#!/bin/sh
# basic functionality check

addwrite $PWD

touch file || exit 1
link-0 0 file link || exit 1
[ -e file -a -e link ]
