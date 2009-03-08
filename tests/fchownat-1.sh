#!/bin/sh
# make sure symlinks with fchownat(AT_SYMLINK_NOFOLLOW) work properly

addwrite $PWD

rm -rf deny link
mkdir deny
touch deny/file
ln -s deny/file link

adddeny $PWD/deny

# this should not fail
exec fchownat-0 0 AT_FDCWD link ${SB_UID} ${SB_GID} AT_SYMLINK_NOFOLLOW
