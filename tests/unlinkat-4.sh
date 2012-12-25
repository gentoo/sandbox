#!/bin/sh
# verify unlinkat("") returns ENOENT in various ways #346929

set -e
unlinkat-0 -1,ENOENT .:O_DIRECTORY '' 0

unlinkat-0 -1,ENOENT -3 '' 0
