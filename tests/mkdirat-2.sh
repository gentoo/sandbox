#!/bin/sh
# verify dirfd handling in pre-checks #342983

addwrite $PWD

# create a dir named "one" in $PWD, then try to create
# a dir named "one" in "one/".  so ultimately we will
# have "one/one/".  if the dirfd is ignored, sandbox
# will return too soon saying "one" already exists.
mkdir one
exec mkdirat-0 0 one:O_DIRECTORY one 0
