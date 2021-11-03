#!/bin/sh
# Make sure forked children are caught.  Historically, dynamic worked fine, but
# static missed forks.
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

# Setup scratch path.
mkdir subdir
adddeny "${PWD}/subdir"

for child in 0 1 2 3 4 5 ; do
	fork-follow_tst ${child} subdir/dyn${child} || exit $?
done

depth="0"
# We can't trace static children currently with YAMA ptrace_scope 1+.
if [ ${at_yama_ptrace_scope} -eq 0 ] ; then
	depth="${depth} 1 2 3 4 5"
fi
for child in ${depth} ; do
	fork-follow_static_tst ${child} subdir/static${child} || exit $?
done

exit 0
