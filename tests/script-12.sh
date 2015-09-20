#!/bin/sh
# handle targets of dangling symlinks correctly #540828
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

# this should pass
mkdir subdir
ln -s subdir/target symlink

# make sure the log is in a writable location
SANDBOX_LOG="${PWD}/subdir/log"

(
# This clobbers all existing writable paths for this one write.
SANDBOX_WRITE="${PWD}/subdir"
echo pass >symlink
)
# we should be able to write through the symlink
if [ $? -ne 0 ] ; then
	exit 1
fi

# and not gotten a sandbox violation
test ! -s "${SANDBOX_LOG}"

exit $?
