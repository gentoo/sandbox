#!/bin/bash

# Generate Changelog by default if we in svn repository
if [[ -d .svn && $1 != -n ]] ; then
	# For some reason svn do not give the full log if we do not
	# update first ...
	svn update
	./scripts/svn2cl.sh
fi

aclocal-1.9 || exit 1
libtoolize --automake -c -f || exit 1
aclocal-1.9 || exit 1
autoconf || exit 1
autoheader || exit 1
automake-1.9 -a -c || exit 1

if [[ -x ./test.sh ]] ; then
	exec ./test.sh "$@"
fi
