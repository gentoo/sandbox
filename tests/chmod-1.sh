#!/bin/sh
# make sure symlinks with chmod() work properly

addwrite $PWD

rm -rf deny link sb.log
mkdir deny
touch deny/file
ln -s deny/file link

# this *should not* trigger a sandbox violation
chmod-0 0 link 0666 || exit 1

# this *should* trigger a sandbox violation
adddeny $PWD/deny
export SANDBOX_LOG=$PWD/sb.log
unset SANDBOX_VERBOSE
chmod-0 -1 link 0666 || exit 1
test -s sb.log
