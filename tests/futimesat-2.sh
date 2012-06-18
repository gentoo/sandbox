#!/bin/sh
# basic functionality check (violations are caught)

adddeny $PWD
futimesat-0 -1,EACCES AT_FDCWD . NULL || exit 1
test -s sandbox.log
