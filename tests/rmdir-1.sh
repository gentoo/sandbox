#!/bin/sh
# basic functionality check

addwrite $PWD

mkdir a || exit 1
rmdir-0 0 a || exit 1
[ ! -e a ]
