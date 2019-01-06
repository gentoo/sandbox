#!/bin/sh
# Make sure execv run does not corrup 'environ' of caller process:
# https://bugs.gentoo.org/669702
timeout -s KILL 10 execv-0
