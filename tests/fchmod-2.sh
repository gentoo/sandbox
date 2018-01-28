#!/bin/sh
#
# Ensure that fchmod() doesn't trigger spurious violations in the most
# basic of cases.
#
addwrite $PWD

# This should not trigger a violation.
rm -f file
touch file
fchmod-0 0644 file || exit 1
