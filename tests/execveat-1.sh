#!/bin/sh
set -e
sb_true=$(command -v sb_true)
test -n "${sb_true}"
dir=$(dirname "${sb_true}")
execveat-0 0 AT_FDCWD "${sb_true}" 0 \
	0 "${dir}:O_PATH" sb_true 0 \
	-1,ENOENT AT_FDCWD badcommand 0
adddeny "${sb_true}"
execveat-0 -1,EACCES AT_FDCWD "${sb_true}" 0 \
	-1,EACCES "${dir}:O_PATH" sb_true 0
