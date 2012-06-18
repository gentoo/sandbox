#!/bin/sh
# make sure symlinks with chmod() work properly

addwrite $PWD

rm -rf deny link
(
set -e
mkdir deny
touch deny/file
ln -s deny/file link
) || exit 1

# this *should not* trigger a sandbox violation
chmod-0 0 link 0666 || exit 1

# this *should* trigger a sandbox violation
adddeny $PWD/deny
chmod-0 -1 link 0666 || exit 1
test -s sandbox.log
