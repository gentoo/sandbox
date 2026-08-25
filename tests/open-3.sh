#!/bin/sh
# make sure the basename survives canonicalization when the target is created
# through a dangling symlink whose parent is itself a symlink

addwrite $PWD

rm -rf protected dirlink filelink
(
set -e
mkdir protected
# the canonical dir name must be longer than the link used to reach it
ln -s protected dirlink
ln -s dirlink/tgt filelink
) || exit 1

# this *should* trigger a sandbox violation
adddeny $PWD/protected/tgt
open-0 -1,EACCES filelink 'O_WRONLY|O_CREAT' 0666 || exit 1
test ! -e protected/tgt
test -s sandbox.log
