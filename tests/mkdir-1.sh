#!/bin/sh
# make sure `mkdir` works in an unreadable dir as non-root.
# based on mkdir/p-3 test from coreutils.

[ ${SB_UID} -eq 0 ] && exit 77

addwrite $PWD

chmod -R a+rwx base 2>/dev/null
rm -rf base

set -e

mkdir -p base/d
cd base/d
chmod a-r .
chmod a-rx ..

mkdir-0 0 a 0777
mkdir-0 0 a/b 0777

chmod a+rx ..
chmod a+r .

exit 0
