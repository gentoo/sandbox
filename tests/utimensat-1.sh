#!/bin/sh
# basic functionality check

addwrite $PWD

touch -r / file || exit 1
utimensat-0 0 AT_FDCWD . NULL 0 || exit 1
utimensat-0 0 AT_FDCWD file NULL 0 || exit 1
[ file -nt / ]
