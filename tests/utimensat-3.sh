#!/bin/sh
# make sure NULL filename is handled correctly

addwrite $PWD

exec utimensat-0 -1:22 'f:O_WRONLY|O_CREAT:0666' NULL NULL 0
