#!/bin/sh
# https://bugs.gentoo.org/139591
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0
addwrite $PWD

mkdir -p to-be/deleted
cd to-be/deleted
rmdir ../deleted

# In https://bugs.gentoo.org/590084 sanbox should deny
# access here and touch should fail:
! touch ../foo
