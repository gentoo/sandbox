#!/bin/bash -e

# we'll generate ChangeLog when doing `make dist`
touch ChangeLog

# avoid ugly warnings due to mismatch between local libtool and
# whatever updated version is on the host
find m4/*.m4 '!' -name 'ax_*.m4' -delete 2>/dev/null

# not everyone has sys-devel/autoconf-archive installed
for macro in $(grep -o '\<AX[A-Z_]*\>' configure.ac | sort -u) ; do
	if m4=$(grep -rl "\[${macro}\]" /usr/share/aclocal/) ; then
		cp -v $m4 m4/
	fi
done

autoreconf -i -f

if [[ -x ./test.sh ]] ; then
	exec ./test.sh "$@"
fi
