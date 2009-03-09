#!/bin/sh
# make sure paths whose parents don't exist do not get flagged by sandbox
exec open-0 -1,ENOENT /a/b/c/d/e/f/g/no/real/path/so/dont/make/it 'O_WRONLY|O_CREAT' 0666
