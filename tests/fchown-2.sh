#!/bin/sh
#
# Ensure that fchown() doesn't trigger spurious violations in the most
# basic of cases.
#
addwrite $PWD

# This should not trigger a violation.
rm -f file
touch file
fchown-0 ${SB_UID} ${SB_GID} file || exit 1
