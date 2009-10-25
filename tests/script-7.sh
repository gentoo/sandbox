#!/bin/sh
# do not barf on fd's that are actually pipes #288863
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

echo | touch /dev/stdin
