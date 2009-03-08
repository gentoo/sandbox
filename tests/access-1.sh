#!/bin/sh
# We should not trigger sandbox on write requests with access()

addwrite $PWD

export SANDBOX_LOG=$PWD/sb.log
access-0 -1 rwx / || exit 1
test ! -e sb.log
