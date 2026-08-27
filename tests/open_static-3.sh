#!/bin/sh
# make sure canonicalizing a dangling symlink does not leak the tracer's fd
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

addwrite $PWD
mkdir deny || exit 1
adddeny $PWD/deny

# stays dangling however often it is opened, as nodir/ never shows up
ln -s nodir/tgt dangle || exit 1

# keep the loop below well over this so the tracer only runs out of
# descriptors if it leaks one per check
ulimit -n 128 || exit 1

set --
i=0
while [ $i -lt 300 ] ; do
	set -- "$@" -1,ENOENT dangle "O_WRONLY|O_CREAT" 0666
	i=$((i + 1))
done

# the write has to stay denied no matter how many checks came before it
open_static-0 "$@" -1,EPERM deny/not-ok "O_WRONLY|O_CREAT" 0666 || exit 1
test ! -e deny/not-ok
