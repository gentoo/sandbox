#!/bin/sh
# basic functionality check
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

addwrite $PWD

touch -r / file || exit 1
utimensat_static-0 0 AT_FDCWD . NULL 0 || exit 1
utimensat_static-0 0 AT_FDCWD file NULL 0 || exit 1
[ file -nt / ]
