#!/bin/sh
# make sure long paths are handled correctly

addwrite $PWD

f="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
mkdir $f
rm -f $f $f/$f
touch $f/$f
exec unlinkat-0 0 $f:O_DIRECTORY $f 0
