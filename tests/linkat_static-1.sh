#!/bin/sh
# basic functionality check
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

addwrite $PWD

touch file || exit 1
linkat_static-0 0 AT_FDCWD file AT_FDCWD link 0 || exit 1
[ -e file -a -e link ]
