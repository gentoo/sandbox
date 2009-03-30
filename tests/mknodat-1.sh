#!/bin/sh
# basic functionality check

addwrite $PWD

exec mknodat-0 0 AT_FDCWD node 0666 0
