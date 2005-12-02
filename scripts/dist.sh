#!/bin/bash

if [[ ! -d .svn ]] ; then
	echo "Not in svn repo!"
	exit 1
fi

set -e

rm -f sandbox-*.tar.bz2
svn update
./autogen.sh
./configure
make dist
scp sandbox-*.tar.bz2 dev.gentoo.org:~/public_html/sandbox/
scp sandbox-*.tar.bz2 dev.gentoo.org:/space/distfiles-local/
rm -f sandbox-*.tar.bz2

