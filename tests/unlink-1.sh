#!/bin/sh
# make sure we catch openat with relative path
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

adddeny "${PWD}"
unlink-0 -1,ENOENT f
