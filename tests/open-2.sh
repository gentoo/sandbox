#!/bin/sh
# make sure we handle open(O_NOFOLLOW) correctly #413441

set -e
rm -f tgt
ln -s tgt sym
open-0 -1,ELOOP sym O_NOFOLLOW 0

touch tgt
open-0 -1,ELOOP sym O_NOFOLLOW 0
