#!/bin/bash -e

# we'll generate ChangeLog when doing `make dist`
touch ChangeLog

# not everyone has sys-devel/autoconf-archive installed
for macro in $(grep -o '\<AX[A-Z_]*\>' configure.ac | sort -u) ; do
	m4=$(grep -rl "\[${macro}\]" /usr/share/aclocal/)
	[[ -z $m4 ]] && continue
	cp -v $m4 m4/
done

autoreconf -i -f

if [[ -x ./test.sh ]] ; then
	exec ./test.sh "$@"
fi
