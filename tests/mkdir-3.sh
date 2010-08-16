#!/bin/sh
# make sure we cannot break out via a symlink in a dir that is
# otherwise not readable as non-root

[ ${SB_UID} -eq 0 ] && exit 77

addwrite $PWD

chmod -R a+rwx base 2>/dev/null
rm -rf base

export SANDBOX_LOG=$PWD/sb.log

set -e

mkdir -p base/d
cd base/d
chmod a-r .
chmod a-rx ..
ln -s / root

# this should trigger a sb violation
unset SANDBOX_PREDICT
(mkdir-0 -1 root/aksdfjasdfjaskdfjasdfla 0777)

chmod a+rx ..
chmod a+r .

test -s "${SANDBOX_LOG}"

exit 0
