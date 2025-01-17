ACLOCAL_AMFLAGS  = -I m4
SIXTY_FOUR_FLAGS = -D_FILE_OFFSET_BITS=64 -D_TIME_BITS=64
MAKEFLAGS = --no-print-directory
AM_CPPFLAGS = \
	-U_FILE_OFFSET_BITS -U_TIME_BITS \
	$(SANDBOX_DEFINES) \
	-I$(top_srcdir)

confdir = $(sysconfdir)
confddir = $(sysconfdir)/sandbox.d

bin_PROGRAMS =
check_PROGRAMS =
dist_conf_DATA = etc/sandbox.conf
confd_DATA = etc/sandbox.d/00default
dist_pkgdata_DATA  = data/sandbox.bashrc
lib_LTLIBRARIES =
noinst_LTLIBRARIES =

CLEANFILES =
DISTCLEANFILES =

EXTRA_DIST = \
	config.h \
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
include tests/local.mk

DISTCLEANFILES += $(CLEANFILES)
