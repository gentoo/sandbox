#!/bin/sh
# make sure NULL filename is handled correctly

addwrite $PWD

exec futimesat-0 0 'f:O_WRONLY|O_CREAT:0666' NULL NULL
