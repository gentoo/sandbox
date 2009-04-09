#!/bin/sh
# make sure we catch openat with relative path
[ "${at_xfail}" = "yes" ] && exit 77 # see trace-0

# if a traced child dies due to a signal, we should pass that
# signal value back up

# POSIX dictates the numeric value of some signals, so let's just
# go with those.  the actual number tested is the way the shell
# works -- 128 + signal number.

EXIT=-128 # hack to negate the 128 base -- we should exit with 0
HUP=1
INT=2
QUIT=3
ABRT=6
KILL=9
ALRM=14
TERM=15
for sig in EXIT HUP INT QUIT ABRT ALRM TERM ; do #KILL
	signum=`eval echo \$sig`
	signam="SIG${sig}"

	printf "testing %s ... " "${signam}"
	signal_static-0 ${signam}
	test $? -eq $((128 + signum)) || exit 1
	echo "OK"
done

exit 0
