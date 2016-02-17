#!/bin/sh
# Since the malloc binary is in the target ABI, make sure the exec is
# launched from the same ABI so the same libsandbox.so is used.
timeout -s KILL 10 execvp-0 malloc_mmap_tst malloc_mmap_tst
