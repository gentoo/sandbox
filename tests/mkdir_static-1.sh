#!/bin/sh
# traced funcs need to go through wrapper pre-checks too #265885
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

exec mkdir_static-0 -1,EEXIST / 0755
