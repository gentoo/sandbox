#!/bin/sh
# Make sure we catch removexattr.
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0
xattr-0 ; ret=$? ; [ ${ret} -eq 0 ] || exit ${ret}

# Set it to something to make sure it works.
touch f
setxattr-0 0 f user.sandbox test 4 0 || exit 1

# Try to remove it and get rejected.
adddeny "${PWD}"
removexattr-0 0 f user.sandbox
test -e sandbox.log
