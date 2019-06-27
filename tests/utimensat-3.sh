#!/bin/sh
# make sure NULL filename is handled correctly

# Note: this test is dependent on glibc internals
# other libcs chose not to validate invalid parameters:
#    https://bugs.gentoo.org/549108#c28
#    https://www.openwall.com/lists/musl/2019/06/25/1
# Run this test only on glibc systems.

case $HOST in
    *-linux-gnu);;
    *) exit 77;;
esac

addwrite $PWD

exec utimensat-0 -1:22 'f:O_WRONLY|O_CREAT:0666' NULL NULL 0
