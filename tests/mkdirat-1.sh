#!/bin/sh
# basic functionality check

addwrite $PWD

exec mkdirat-0 0 AT_FDCWD dir 0777
