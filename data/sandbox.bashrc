# Copyright (C) 2001 Geert Bevin, Uwyn, http://www.uwyn.com
# Distributed under the terms of the GNU General Public License, v2 or later 
# Author : Geert Bevin <gbevin@uwyn.com>
# $Header$
source /etc/profile

if [[ -n ${LD_PRELOAD} && ${LD_PRELOAD} != *$SANDBOX_LIB* ]] ; then
	export LD_PRELOAD="${SANDBOX_LIB} ${LD_PRELOAD}"
elif [[ -z ${LD_PRELOAD} ]] ; then
	export LD_PRELOAD="${SANDBOX_LIB}"
fi

export BASH_ENV="${SANDBOX_BASHRC}"

alias make="make LD_PRELOAD=${LD_PRELOAD}"
alias su="su -c '/bin/bash -rcfile ${SANDBOX_BASHRC}'"

declare -r SANDBOX_ACTIVE
