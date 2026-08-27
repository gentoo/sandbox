#!/bin/sh
# make sure the tracer does not leak a /proc fd per checked syscall
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

addwrite $PWD
mkdir deny || exit 1
adddeny $PWD/deny

# keep the loop below well over this so the tracer only runs out of
# descriptors if it leaks one per check
ulimit -n 128 || exit 1

set --
i=0
while [ $i -lt 300 ] ; do
	set -- "$@" -1,ENOENT "nope-$i/file" O_RDONLY 0
	i=$((i + 1))
done

# the write has to stay denied no matter how many checks came before it
open_static-0 "$@" -1,EPERM deny/not-ok "O_WRONLY|O_CREAT" 0666 || exit 1
test ! -e deny/not-ok
