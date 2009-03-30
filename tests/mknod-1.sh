#!/bin/sh
# basic functionality check

addwrite $PWD

exec mknod-0 0 node 0666 0
