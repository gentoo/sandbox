#!/bin/sh
# make sure poking remote addresses works even when they're bad/unaligned #560396
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

trace-memory_static_tst
