#!/bin/bash

for f in \
	`find . -name Makefile.in -o -name Makefile` \
	`find . -name .libs -o -name .deps -type d` \
	`find . -name '*.o' -o -name '*.la' -o -name '*.lo' -o -name '*.loT'` \
	aclocal.m4* autom4te.cache \
	configure config.* \
	depcomp install-sh ltmain.sh missing mkinstalldirs libtool \
	compile sandbox stamp-* ;
do
	rm -rf $f
done
