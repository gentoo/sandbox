#!/bin/sh
# verify openat("") returns ENOENT in various ways #346929

set -e
openat-0 -1,ENOENT .:O_DIRECTORY '' O_RDONLY 0
openat-0 -1,ENOENT .:O_DIRECTORY '' 'O_CREAT|O_WRONLY' 0

openat-0 -1,ENOENT -3 '' O_RDONLY 0
openat-0 -1,ENOENT -3 '' 'O_CREAT|O_WRONLY' 0
