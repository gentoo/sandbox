# Copyright (C) 2001 Geert Bevin, Uwyn, http://www.uwyn.com
# Distributed under the terms of the GNU General Public License, v2 or later 

if [[ -n ${LD_PRELOAD} && ${LD_PRELOAD} != *$SANDBOX_LIB* ]] ; then
	export LD_PRELOAD="${SANDBOX_LIB} ${LD_PRELOAD}"
elif [[ -z ${LD_PRELOAD} ]] ; then
	export LD_PRELOAD="${SANDBOX_LIB}"
fi

export BASH_ENV="${SANDBOX_BASHRC}"

alias make="make LD_PRELOAD=${LD_PRELOAD}"
alias su="su -c '/bin/bash -rcfile ${SANDBOX_BASHRC}'"

declare -r SANDBOX_ACTIVE

# Only do Mike's sandboxshell mojo if we are interactive, and if
# we are connected to a terminal (ie, not piped, etc)
if [[ ${SANDBOX_INTRACTV} == "1" && -t 1 ]] || [[ ${__SANDBOX_TESTING} == "yes" ]] ; then
	trap ":" INT QUIT TSTP

	# Make sure this do not get recusively called
	unset SANDBOX_INTRACTV

	# Do not set this, as user might want to override path, etc ... #139591
	#source /etc/profile

	if [[ ${__SANDBOX_TESTING} != "yes" ]] ; then
		(
		[[ ${NOCOLOR} == "true" || ${NOCOLOR} == "yes" || ${NOCOLOR} == "1" ]] && \
			export RC_NOCOLOR="yes"
		source /etc/init.d/functions.sh
		if [ $? -ne 0 ] ; then
			einfo() { echo " INFO: $*"; }
			ewarn() { echo " WARN: $*"; }
			eerror() { echo " ERR: $*"; }
		fi
		echo
		einfo "Loading sandboxed shell"
		einfo " Log File:           ${SANDBOX_LOG}"
		if [[ -n ${SANDBOX_DEBUG_LOG} ]] ; then
			einfo " Debug Log File:     ${SANDBOX_DEBUG_LOG}"
		fi
		einfo " sandboxon:          turn sandbox on"
		einfo " sandboxoff:         turn sandbox off"
		einfo " addread <path>:     allow <path> to be read"
		einfo " addwrite <path>:    allow <path> to be written"
		einfo " adddeny <path>:     deny access to <path>"
		einfo " addpredict <path>:  allow fake access to <path>"
		echo
		)
	fi

	# do ebuild environment loading ... detect if we're in portage
	# build area or not ... uNF uNF uNF
	#sbs_pdir=$(portageq envvar PORTAGE_TMPDIR)/portage/ #portageq takes too long imo
	if [[ -z ${PORTAGE_TMPDIR} ]] ; then
		sbs_gpdir=$( source /etc/make.globals && echo $PORTAGE_TMPDIR 2> /dev/null)
		sbs_cpdir=$( source /etc/make.conf && echo $PORTAGE_TMPDIR 2> /dev/null)
		[[ -z ${sbs_cpdir} ]] \
			&& sbs_pdir=${sbs_gpdir} \
			|| sbs_pdir=${sbs_cpdir}
	else
		sbs_pdir=${PORTAGE_TMPDIR}
	fi
	[[ -z ${sbs_pdir} ]] && sbs_pdir=/var/tmp
	sbs_pdir=${sbs_pdir}/portage/

	if [[ ${PWD:0:${#sbs_pdir}} == "${sbs_pdir}" ]] ; then
		sbs_bdir=$(echo ${PWD:${#sbs_pdir}} | cut -d/ -f1)
		sbs_tmpenvfile=${sbs_pdir}${sbs_bdir}/temp/environment
		if [[ -e ${sbs_tmpenvfile} ]] ; then
			echo "Found environment at ${sbs_tmpenvfile}"
			printf " * Would you like to enter the portage environment ? "
			read env
			sbs_PREPWD=${PWD}
			if [[ ${env} == "y" ]] ; then
				# First try to source variables and export them ...
				eval $(sed -e '/^[[:alnum:]_-]*=/s:^:export :' \
				           -e '/^[[:alnum:]_-]* ()/Q' "${sbs_tmpenvfile}") 2>/dev/null
				# Then grab everything (including functions)
				source "${sbs_tmpenvfile}" 2> /dev/null
				export SANDBOX_WRITE=${SANDBOX_WRITE}:${sbs_pdir}${sbs_bdir}:${sbs_pdir}/homedir
			fi
			PWD=${sbs_PREPWD}
		fi
	fi

	unset sbs_gpdir sbs_cpdir sbs_pdir sbs_bdir sbs_tmpenvfile sbs_PREPWD env

	cd "${PWD}"
	if [[ ${NOCOLOR} != "true" && ${NOCOLOR} != "yes" && ${NOCOLOR} != "1" ]] ; then
		export PS1="\[\e[31;01m\][s]\[\e[0m\] ${PS1}"
	else
		export PS1="[s] ${PS1}"
	fi

	adddeny()    { export SANDBOX_DENY=${SANDBOX_DENY}:$1 ; }
	addpredict() { export SANDBOX_PREDICT=${SANDBOX_PREDICT}:$1 ; }
	addread()    { export SANDBOX_READ=${SANDBOX_READ}:$1 ; }
	addwrite()   { export SANDBOX_WRITE=${SANDBOX_WRITE}:$1 ; }
	sandboxon()  { export SANDBOX_ON="1" ; }
	sandboxoff() { export SANDBOX_OFF="0" ; }

	[[ -z ${CCACHE_DIR} ]] && [[ -w /root/.ccache ]] && export CCACHE_DIR=/root/.ccache
	for var in CCACHE_DIR DISTCC_DIR ; do
		[[ ${!var+set} == "set" ]] && addwrite ${!var}
	done
	unset var
fi
