#!/bin/sh
# make sure we don't accidentally trip atime updates on files
# through symlinks #415475
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

# We assume $PWD supports atimes, and the granularity is more than 1 second.
# If it doesn't, this test will still pass, but not really because the code
# was proven to be correct.

# XXX: Maybe we need to add our own stat shim to avoid portability issues ?
get_atime() {
	# This shows the full atime field (secs, msecs, nsecs).
	stat -c %x "$1"
}

# Create a symlink.
sym="sym"
ln -s atime "${sym}"

# Get the state before we test it.
before=$(get_atime "${sym}")

# A quick sleep of a few msecs.
sleep 0.1

# See if the atime changes -- it should not.
utimensat-0 -1,EINVAL AT_FDCWD "${sym}" -1,-1 AT_SYMLINK_NOFOLLOW || exit 1
after=$(get_atime "${sym}")

[ "${after}" = "${before}" ]
