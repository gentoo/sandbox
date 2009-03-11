#!/bin/sh
# make sure LD_PRELOAD makes it back into env on exec
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

checkit() {
	if ! eval "$@" | grep -q '^LD_PRELOAD=.*libsandbox.so.*' ; then
		echo "LD_PRELOAD was not reset when running:"
		echo "	$*"
		exit 1
	fi
}

checkit "env -uLD_PRELOAD env"
checkit "(unset LD_PRELOAD; env)"
checkit "env LD_PRELOAD= env"
checkit "env LD_PRELOAD=libc.so env"
exit 0
