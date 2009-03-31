#!/bin/sh
# make sure `mkdir -p /var` does not trigger sb violations
# since the paths should already exist
exec mkdir-0 -1,EEXIST / 0777
