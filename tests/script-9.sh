#!/bin/sh
# verify tracing static apps with pipes #364877
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

pipe-fork_tst
