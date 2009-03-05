#!/bin/sh
# http://bugs.gentoo.org/260765
addwrite $PWD

cat << EOF > Makefile
all:
	rm -rf .libs
	mkdir .libs
	rm -rf .libs
	printf OK
EOF

# autotest sets MAKEFLAGS on us
unset MAKEFLAGS

exec make -s
