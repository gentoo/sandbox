#!/bin/sh
# make sure all the SANDBOX env vars make it back in.
[ "${at_xfail}" = "yes" ] && exit 77 # see script-0

ret=0

out=$(env -i env)
for var in LOG DEBUG_LOG MESSAGE_PATH DENY READ WRITE PREDICT ON ACTIVE ; do
	var="SANDBOX_${var}"
	oval=$(env | grep "^${var}=" | sed 's:^[^=]*=::')

	nval=$(echo "${out}" | sed -n "/^${var}=/s:[^=]*=::p")

	[ "${nval}" != "${oval}" ] && echo "!!! MISMATCH !!!" && ret=1
	echo "env  [${var}]='${oval}'"
	echo "env-i[${var}]='${nval}'"
	[ "${nval}" != "${oval}" ] && echo "!!! MISMATCH !!!"
	echo
done

exit ${ret}
