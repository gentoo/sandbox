ACLOCAL_AMFLAGS  = -I m4
AUTOMAKE_OPTIONS = dist-lzma no-dist-gzip

SUBDIRS = \
	scripts    \
	etc        \
	data       \
	libsbutil  \
	libsandbox \
	src        \
	tests

libsandbox: libsbutil
src: libsbutil
tests: src

EXTRA_DIST = headers.h localdecls.h ChangeLog.0

ChangeLog:
	touch ChangeLog

dist-hook:
	set -e ; \
	if [ -d "$(top_srcdir)/.git" ] ; then \
		git --git-dir="$(top_srcdir)/.git" log > $(distdir)/ChangeLog ; \
	elif [ ! -e "$(distdir)/ChangeLog" ] ; then \
		if [ -e "$(top_srcdir)/ChangeLog" ] ; then \
			cp "$(top_srcdir)/ChangeLog" "$(distdir)/ChangeLog" ; \
		else \
			touch "$(distdir)/ChangeLog" ; \
		fi ; \
	fi
