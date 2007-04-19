#!/bin/bash -e

# we'll generate ChangeLog when doing `make dist`
touch ChangeLog

autoreconf -i -f

if [[ -x ./test.sh ]] ; then
	exec ./test.sh "$@"
fi
