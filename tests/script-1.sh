#!/bin/sh
# https://bugs.gentoo.org/257418
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0
(
cd "${devfd}"
>3
)
exit 0
