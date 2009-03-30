#!/bin/sh
# basic functionality check

addwrite $PWD

touch old
rename-0 0 old new || exit 1
[ ! -e old -a -e new ]
