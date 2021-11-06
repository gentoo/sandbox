#!/bin/bash -e

# we'll generate ChangeLog when doing `make dist`
touch ChangeLog

# avoid ugly warnings due to mismatch between local libtool and
# whatever updated version is on the host
rm -f m4/*.m4

# not everyone has sys-devel/autoconf-archive installed
has() { [[ " ${*:2} " == *" $1 "* ]] ; }
import_ax() {
	local macro content m4 found lm4s=()
	content=$(sed -e '/^[[:space:]]*#/d' -e 's:\<dnl\>.*::' "$@")
	for macro in $(echo "${content}" | grep -o '\<AX[A-Z_]*\>' | sort -u) ; do
		if ! found=$(grep -rl "AC_DEFUN(\[${macro}\]" /usr/share/aclocal/) ; then
			echo "error: ${macro}: unable to locate m4 definition"
			exit 1
		fi
		for m4 in ${found} ; do
			if ! has ${m4} "${m4s[@]}" "${lm4s[@]}" ; then
				echo "$*: ${macro}: ${m4}"
				lm4s+=( ${m4} )
			fi
		done
	done
	if [[ ${#lm4s[@]} -gt 0 ]] ; then
		cp -v `printf '%s\n' ${lm4s[@]} | sort -u` m4/
		m4s+=( "${lm4s[@]}" )
	fi
}
m4s=()
import_ax configure.ac
curr=1
new=0
while [[ ${curr} -ne ${new} ]] ; do
	curr=${#m4s[@]}
	import_ax m4/ax_*.m4
	new=${#m4s[@]}
done

autoreconf -i -f

if [[ -x ./test.sh ]] ; then
	exec ./test.sh "$@"
fi
