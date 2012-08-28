ACLOCAL_AMFLAGS  = -I m4
MAKEFLAGS = --no-print-directory
AM_CPPFLAGS = $(SANDBOX_DEFINES)

SUBDIRS = \
	scripts    \
	etc        \
	data       \
	libsbutil  \
	libsandbox \
	src        \
	tests

noinst_LTLIBRARIES =

SANDBOX_PCH = headers.h.gch libsandbox/headers.h.gch libsbutil/headers.h.gch
nodist_libpch_la_SOURCES = $(SANDBOX_PCH)
GCH_CP = ( \
	src=`dirname $@`/.libs/`basename $@`.o; \
	test -e "$$src" || src=$@.o; \
	cp $$src $@ \
	)
# This is built as PIC
$(builddir)/libsbutil/headers.h.gch: headers.h
	$(AM_V_GEN)$(LTCOMPILE) -c -o $@.o $< && $(GCH_CP)
# This is built as PIC + exceptions
$(builddir)/libsandbox/headers.h.gch: headers.h
	$(AM_V_GEN)$(LTCOMPILE) -c -o $@.o $< $(CFLAG_EXCEPTIONS) && $(GCH_CP)
# This is built w/out PIC
$(builddir)/headers.h.gch: headers.h
	$(AM_V_GEN)$(COMPILE) -c -o $@.o $< && $(GCH_CP)

if SB_BUILD_PCH
BUILT_SOURCES = $(SANDBOX_PCH)
noinst_LTLIBRARIES += libpch.la

LIBSBUTIL_PCH = libsbutil/headers.h.gch
LIBSANDBOX_PCH = libsandbox/headers.h.gch
TOP_PCH = headers.h.gch

# Make sure we build the subdirs before the top so they don't
# try to use the top level headers.h.pch.
$(TOP_PCH): $(LIBSBUTIL_PCH) $(LIBSANDBOX_PCH)
endif

libsbutil: $(LIBSBUTIL_PCH)
libsandbox: libsbutil $(LIBSANDBOX_PCH)
src: libsbutil $(TOP_PCH)
tests: src $(TOP_PCH)

EXTRA_DIST = headers.h localdecls.h ChangeLog.0

CLEANFILES = $(SANDBOX_PCH)
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
