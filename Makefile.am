ACLOCAL_AMFLAGS  = -I m4
MAKEFLAGS = --no-print-directory
AM_CPPFLAGS = \
	$(SANDBOX_DEFINES) \
	-I$(top_srcdir)

SUBDIRS = tests

confdir = $(sysconfdir)
confddir = $(sysconfdir)/sandbox.d

bin_PROGRAMS =
dist_conf_DATA = etc/sandbox.conf
confd_DATA = etc/sandbox.d/00default
dist_pkgdata_DATA  = data/sandbox.bashrc
lib_LTLIBRARIES =
noinst_LTLIBRARIES =

CLEANFILES =

EXTRA_DIST = \
	headers.h \
	localdecls.h \
	ChangeLog.0 \
	data/sandbox.desktop \
	data/sandbox.svg \
	etc/sandbox.d/00default.in \
	scripts/gen_symbol_version_map.awk \
	scripts/gen_symbol_header.awk \
	scripts/gen_trace_header.awk

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

include libsandbox/local.mk
include libsbutil/local.mk
include src/local.mk

DISTCLEANFILES = $(CLEANFILES)
