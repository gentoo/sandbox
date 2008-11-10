AUTOMAKE_OPTIONS = dist-lzma no-dist-gzip

SUBDIRS = \
	scripts    \
	etc        \
	data       \
	libsbutil  \
	libsandbox \
	src

libsandbox: libsbutil
src: libsbutil

EXTRA_DIST = headers.h localdecls.h ChangeLog.0

dist-hook:
	set -e ; \
	if [ -d "$(top_srcdir)/.svn" -a -x "$(top_srcdir)/scripts/svn2cl" ] ; then \
		PATH="$${PATH}:$(top_srcdir)/scripts" svn2cl ; \
		cp ChangeLog $(distdir)	; \
	elif [ ! -e "$(distdir)/ChangeLog" ] ; then \
		if [ -e "$(top_srcdir)/ChangeLog" ] ; then \
			cp "$(top_srcdir)/ChangeLog" "$(distdir)/ChangeLog" ; \
		else \
			touch "$(distdir)/ChangeLog" ; \
		fi ; \
	fi
