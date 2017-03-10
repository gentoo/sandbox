#!/bin/sh
# make sure we can clobber symlinks #612202

addwrite $PWD

symlinkat-0 0 /asdf AT_FDCWD ./sym || exit 1
[ -L sym ]
symlinkat-0 -1,EEXIST /asdf AT_FDCWD ./sym || exit 1
[ -L sym ]
test ! -s "${SANDBOX_LOG}"
