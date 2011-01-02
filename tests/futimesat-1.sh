#!/bin/sh
# basic functionality check

addwrite $PWD

touch -r / file || exit 1
futimesat-0 0 AT_FDCWD . NULL || exit 1
futimesat-0 0 AT_FDCWD file NULL || exit 1
[ file -nt / ]
