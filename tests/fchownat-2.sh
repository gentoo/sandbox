#!/bin/sh
# make sure symlinks with fchownat() work properly

addwrite $PWD

rm -rf deny link
mkdir deny
touch deny/file
ln -s deny/file link

# this *should not* trigger a sandbox violation
fchownat-0 0 AT_FDCWD link ${SB_UID} ${SB_GID} 0 || exit 1

# this *should* trigger a sandbox violation
adddeny $PWD/deny
export SANDBOX_LOG=$PWD/sb.log
unset SANDBOX_VERBOSE
fchownat-0 -1 AT_FDCWD link ${SB_UID} ${SB_GID} 0 || exit 1
test -s sb.log
