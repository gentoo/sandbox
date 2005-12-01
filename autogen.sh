#!/bin/bash

if [[ -d .svn && $1 != -n ]] ; then
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
