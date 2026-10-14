#!/bin/sh
set -e
sb_true=$(command -v sb_true)
test -n "${sb_true}"
execle-0 0 "${sb_true}"
adddeny "${sb_true}"
execle-0 -1,EACCES "${sb_true}"
