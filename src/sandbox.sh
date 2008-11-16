#!/bin/sh
# quick wrapper to run local sandbox with local libsandbox
dir=${0%/*}
export LD_LIBRARY_PATH=${dir}/../libsandbox/.libs
exec "${dir}"/sandbox "$@"
