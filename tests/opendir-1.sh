#!/bin/sh
# check that very long paths to opendir() do not cause segv
path=
for (( i = 0; i < 1000; i++ )); do
	path+=/verylong
done
exec opendir-0 0,ENAMETOOLONG "${path}"
