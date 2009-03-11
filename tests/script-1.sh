#!/bin/sh
# http://bugs.gentoo.org/257418
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0
(>/dev/fd/3)
exit 0
