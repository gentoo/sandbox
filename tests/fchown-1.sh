#!/bin/sh
#
# https://bugs.gentoo.org/599706
#

addwrite $PWD

# The sandbox doesn't log anything when it returns a junk file
# descriptor? It doesn't look like we can test the contents of
# sandbox.log here... instead, we just have to count on fchown
# failing, which it does if you use O_RDWR, and it *should* if you use
# O_RDONLY (because that won't stop the change of ownership).
fchown-0 ${SB_UID} ${SB_GID} ../.. && exit 1
exit 0
