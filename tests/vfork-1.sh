#!/bin/sh
# vfork and trace wierdness #264478

trace-0 ; test $? -eq 77 && exit 77

# the exact binary we run doesnt matter, it just needs to be
# static so that the trace code works
output=$(vfork-0 sb_true_static sb_true_static sb_true_static 2>&1 || echo fail)
echo "$output"
exec test -z "$output"

exit 1
# Below is original test case
[ -e /usr/bin/fpc ] || exit 77

addwrite $PWD

cat <<"EOF" > Makefile
a := $(shell $(shell /usr/bin/fpc -PB) -iVSPTPSOTO)
all:
EOF

output=$(make -s 2>&1)
echo "$output"
test -z "$output"
