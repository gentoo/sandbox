#!/bin/sh

# svn2cl.sh - front end shell script for svn2cl.xsl, calls xsltproc
#             with the correct parameters
# 
# Copyright (C) 2005 Arthur de Jong.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. The name of the author may not be used to endorse or promote
#    products derived from this software without specific prior
#    written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# exit on any failures
set -e

# svn2cl version
VERSION="0.3"

# set default parameters
STRIPPREFIX=`basename $(pwd)`
LINELEN=75
GROUPBYDAY="no"
INCLUDEREV="no"
CHANGELOG="ChangeLog"

# do command line checking
prog=`basename $0`
while [ -n "$1" ]
do
  case "$1" in
    --strip-prefix)
      STRIPPREFIX="$2"
      shift 2
      ;;
    --linelen)
      LINELEN="$2";
      shift 2
      ;;
    --group-by-day)
      GROUPBYDAY="yes";
      shift
      ;;
    -r|--include-rev)
      INCLUDEREV="yes";
      shift
      ;;
    -o|--output)
      CHANGELOG="$2"
      shift 2
      ;;
    --stdout)
      CHANGELOG="-"
      shift
      ;;
    -V|--version)
      echo "$prog $VERSION";
      echo "Written by Arthur de Jong."
      echo ""
      echo "Copyright (C) 2005 Arthur de Jong."
      echo "This is free software; see the source for copying conditions.  There is NO"
      echo "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
      exit 0
      ;;
    -h|--help)
      echo "Usage: $prog [OPTION]..."
      echo "Generate a ChangeLog from a checked out subversion repository."
      echo ""
      echo "  --strip-prefix NAME  prefix to strip from all entries, defaults"
      echo "                       to the name of the current directory"
      echo "  --linelen NUM        maximum length of an output line"
      echo "  --group-by-day       group changelog entries by day"
      echo "  -r, --include-rev    include revision numbers"
      echo "  -o, --output FILE    output to FILE instead of ChangeLog"
      echo "  -f, --file FILE      alias for -o, --output"
      echo "  --stdout             output to stdout instead of ChangeLog"
      echo "  -h, --help           display this help and exit"
      echo "  -V, --version        output version information and exit"
      exit 0
      ;;
    *)
      echo "$prog: invalid option -- $1"
      echo "Try \`$prog --help' for more information."
      exit 1
      ;;
  esac
done

# find the directory that this script resides in
prog="$0"
while [ -h "$prog" ]
do
  prog=`ls -ld "$prog" | sed "s/^.*-> \(.*\)/\1/;/^[^/]/s,^,$(dirname "$prog")/,"`
done
dir=`dirname $prog`
dir=`cd $dir && pwd`
XSL="$dir/svn2cl.xsl"

# redirect stdout to the changelog file if needed
if [ "x$CHANGELOG" != "x-" ]
then
  exec > "$CHANGELOG"
fi

# actually run the command we need
svn --verbose --xml log | \
  xsltproc --stringparam strip-prefix "$STRIPPREFIX" \
           --stringparam linelen $LINELEN \
           --stringparam groupbyday $GROUPBYDAY \
           --stringparam include-rev $INCLUDEREV \
           "$XSL" -
