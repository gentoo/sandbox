ACLOCAL_AMFLAGS  = -I m4
MAKEFLAGS = --no-print-directory
AM_CPPFLAGS = $(SANDBOX_DEFINES)

SUBDIRS = \
	libsbutil  \
	libsandbox \
	src        \
	tests

confdir = $(sysconfdir)
confddir = $(sysconfdir)/sandbox.d
pixmapdir = $(datadir)/pixmaps
desktopdir = $(datadir)/applications

dist_conf_DATA = etc/sandbox.conf
confd_DATA = etc/sandbox.d/00default
dist_pkgdata_DATA  = data/sandbox.bashrc
dist_pixmap_DATA = data/sandbox.svg
dist_desktop_DATA = data/sandbox.desktop
noinst_LTLIBRARIES =

libsandbox: libsbutil
src: libsbutil
tests: src

EXTRA_DIST = \
	headers.h \
	localdecls.h \
	ChangeLog.0 \
	etc/sandbox.d/00default.in \
	scripts/gen_symbol_version_map.awk \
	scripts/gen_symbol_header.awk \
	scripts/gen_trace_header.awk

DISTCLEANFILES = $(CLEANFILES)

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

install-exec-hook:
	set -e ; \
	for f in $(bindir)/sandbox $(libdir)/libsandbox.so ; do \
		sed -i.tmp \
			's:__SANDBOX_TESTING:\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00:' \
			$(DESTDIR)$$f ; \
		rm -f $(DESTDIR)$$f.tmp ; \
	done
