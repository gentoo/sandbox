#!/bin/sh
# make sure we reject bad renames #612202

addwrite $PWD
mkdir deny
adddeny $PWD/deny

touch file
renameat-0 -1,EACCES AT_FDCWD file AT_FDCWD deny/file || exit 1
[ -e file ]
test -s "${SANDBOX_LOG}"
