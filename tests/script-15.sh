#!/bin/sh
# check that sandbox stops cloberring LD_PRELOAD when it's disabled
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

SANDBOX_ON=0

# keep in sync with script-4!
checkit() {
	if eval "$@" | grep -q '^LD_PRELOAD=.*libsandbox.so.*' ; then
		echo "LD_PRELOAD was reset when running (w/ SANDBOX_ON=0):"
		echo "	$*"
		exit 1
	fi
}

checkit "env -uLD_PRELOAD env"
checkit "(unset LD_PRELOAD; env)"
checkit "env LD_PRELOAD= env"
checkit "env LD_PRELOAD=libc.so env"

exit $?
