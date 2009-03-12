#!/bin/sh
# make sure `sandbox` passes exit statuses back up
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

exit 8
