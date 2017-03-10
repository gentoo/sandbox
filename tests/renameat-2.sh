#!/bin/sh
# make sure we can clobber symlinks #612202

addwrite $PWD

ln -s /asdf sym || exit 1
touch file
renameat-0 0 AT_FDCWD file AT_FDCWD sym || exit 1
[ ! -e file ]
[ ! -L sym ]
[ -e sym ]
test ! -s "${SANDBOX_LOG}"
