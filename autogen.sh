#!/bin/bash -e

# we'll generate ChangeLog when doing `make dist`
touch ChangeLog

# avoid ugly warnings due to mismatch between local libtool and
# whatever updated version is on the host
find m4/*.m4 '!' -name 'ax_*.m4' -delete 2>/dev/null || :

# not everyone has sys-devel/autoconf-archive installed
has() { [[ " ${*:2} " == *" $1 "* ]] ; }
import_ax() {
	local macro content m4 lm4s
	content=$(sed -e '/^[[:space:]]*#/d' -e 's:\<dnl\>.*::' "$@")
	for macro in $(echo "${content}" | grep -o '\<AX[A-Z_]*\>' | sort -u) ; do
		if m4=$(grep -rl "\[${macro}\]" /usr/share/aclocal/) ; then
			has ${m4} "${m4s[@]}" || lm4s+=( ${m4} )
		fi
	done
	if [[ ${#lm4s[@]} -gt 0 ]] ; then
		cp -v `printf '%s\n' ${lm4s[@]} | sort -u` m4/
		m4s+=( "${lm4s[@]}" )
	fi
}
m4s=()
import_ax configure.ac
import_ax m4/ax_*.m4
import_ax m4/ax_*.m4

autoreconf -i -f

if [[ -x ./test.sh ]] ; then
	exec ./test.sh "$@"
fi
