#!/bin/sh
# We should not trigger sandbox on write requests with access()

addwrite $PWD

access-0 -1 rwx / || exit 1
test ! -e sandbox.log
