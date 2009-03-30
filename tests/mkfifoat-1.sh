#!/bin/sh
# basic functionality check

addwrite $PWD

exec mkfifoat-0 0 AT_FDCWD fifo 0777
