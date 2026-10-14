#!/bin/sh
set -e

# Check libsandbox ABI against /bin/sh
script-0

# 32512 == (127 << 8)
system-0 0 sb_true 32512 ./badcommand

adddeny "$(command -v sb_true)"
# 32256 == (126 << 8)
system-0 32256 sb_true

adddeny /bin/sh
system-0 -1,EACCES sb_true
