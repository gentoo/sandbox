#!/bin/sh
# verify mkdirat("") returns ENOENT in various ways #346929

set -e
mkdirat-0 -1,ENOENT .:O_DIRECTORY '' 0

mkdirat-0 -1,ENOENT -3 '' 0
