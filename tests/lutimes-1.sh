#!/bin/sh

addwrite "${PWD}"

sym="lutimes-1.sym"
ln -s /bad/path "${sym}"

lutimes-0 0 "${sym}" NULL || exit 1
lutimes-0 -1,EACCES /bin/sh NULL || exit 1
