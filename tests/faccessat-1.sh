#!/bin/sh
set -e
addwrite "$PWD/file"
faccessat-0 0 'file:O_RDWR|O_CREAT:0666' '' rw AT_EMPTY_PATH
exec 9<file
adddeny "$PWD/file"
faccessat-0 -1,EACCES 9 '' rw AT_EMPTY_PATH
