#!/bin/sh
# make sure we reject bad symlinks #612202

addwrite $PWD
mkdir deny
adddeny $PWD/deny

symlinkat-0 -1,EACCES ./ AT_FDCWD deny/sym || exit 1
test -s "${SANDBOX_LOG}"
