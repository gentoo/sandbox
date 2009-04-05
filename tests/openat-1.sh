#!/bin/sh
# make sure unknown fd's have errno set to EBADF

exec openat-0 -1,EBADF -1 foo 0 0666
