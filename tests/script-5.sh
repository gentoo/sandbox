#!/bin/sh
# make sure extra long paths don't cause sandbox to shit itself
# (actual exit status is irrelevant as it depends on host libc)
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

addwrite $PWD
getcwd-gnulib_tst
case $? in
	0|1|2) exit 0;; # test may exit with these values
	*)     echo "someone get a diaper"; exit 1;;
esac
