#!/bin/sh
# Make sure we catch setxattr.
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0
xattr-0 ; ret=$? ; [ ${ret} -eq 0 ] || exit ${ret}

# Set it to something to make sure it works.
touch f
setxattr-0 0 f user.sandbox test 4 0 || exit 1

# Try to set it again and get rejected.
adddeny "${PWD}"
setxattr-0 0 f user.sandbox test 4 0
test -e sandbox.log
