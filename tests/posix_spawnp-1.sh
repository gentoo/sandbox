#!/bin/sh
adddeny /bin/false
exec posix_spawnp-0 0 sb_true EACCES /bin/false ENOENT ./badfile
