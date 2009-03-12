#!/bin/sh
# make sure `mkdir -p /var` does not trigger sb violations
# since the paths should already exist
mkdir-0 0 / 0777
exit 0
