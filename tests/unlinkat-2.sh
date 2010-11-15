#!/bin/sh
# verify dirfd handling in pre-checks #342983

addwrite $PWD

# create a dir named "one" in $PWD, then try to unlink
# a file named "xxxx" in "one/".  so ultimately we will
# have unlink("one/xxxx").  if the dirfd is ignored, sandbox
# will return too soon saying "xxxx" does not exist.
mkdir one
rm -f xxxx one/xxxx
touch one/xxxx
exec unlinkat-0 0 one:O_DIRECTORY xxxx 0
