#!/bin/sh
# handle targets of dangling symlinks correctly #540828
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

# this should fail
mkdir subdir
ln -s subdir/target symlink

adddeny "${PWD}/subdir"

echo blah >symlink
# we should not be able to write through the symlink
if [ $? -eq 0 ] ; then
	exit 1
fi

test -s "${SANDBOX_LOG}"

exit $?
