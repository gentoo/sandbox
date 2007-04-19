AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip

SUBDIRS = \
	scripts    \
	etc        \
	data       \
	libsbutil  \
	libsandbox \
	src

libsandbox: libsbutil
src: libsbutil

EXTRA_DIST = localdecls.h ChangeLog.0

dist-hook:
	PATH="$${PATH}:./scripts" svn2cl
	cp ChangeLog $(distdir)	
