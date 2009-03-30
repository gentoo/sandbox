#!/bin/sh
# basic functionality check

addwrite $PWD

exec mkfifo-0 0 fifo 0777
