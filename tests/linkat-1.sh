#!/bin/sh
# basic functionality check

addwrite $PWD

touch file || exit 1
linkat-0 0 AT_FDCWD file AT_FDCWD link 0 || exit 1
[ -e file -a -e link ]
