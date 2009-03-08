#!/bin/sh
# make sure symlinks with lchown() work properly

addwrite $PWD

rm -rf deny link
mkdir deny
touch deny/file
ln -s deny/file link

adddeny $PWD/deny

# this should not fail
exec lchown-0 0 link ${SB_UID} ${SB_GID}
