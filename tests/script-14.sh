#!/bin/sh
# check that paths don't accidentally match other files by prefix
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

(
# This clobbers all existing writable paths for this one write.
SANDBOX_PREDICT=/dev/null
SANDBOX_WRITE="${PWD}/foo"
echo FAIL >foobar
)
# the write to 'logfoobar' should be rejected since only 'log'
# is supposed to be writable
if [ $? -eq 0 ] ; then
	exit 1
fi

# and we should have gotten a sandbox violation
test -s "${SANDBOX_LOG}"

exit $?
