SUBDIRS = \
	util \
	core

rcdir = $(pkgincludedir)
rc_HEADERS = \
	rctypes.h \
	rcutil.h  \
	rccore.h
nodist_rc_HEADERS = rcdefines.h

rcdefines.h: $(srcdir)/rcdefines.h.in
	@rm -f rcdefines.h
	@echo "Generating rcdefines.h..."
	@sed -e 's:@ETCDIR@:$(sysconfdir):g'        \
	     -e 's:@BINDIR@:$(bindir):g'            \
	     -e 's:@LIBDIR@:$(libdir):g'            \
	     -e 's:@SBINDIR@:$(sbindir):g'          \
	     -e 's:@VARDIR@:$(localstatedir):g'     \
	     -e 's:@SHELL_PARSER@:$(RC_SHELL):g'    \
	     $(srcdir)/rcdefines.h.in > rcdefines.h

EXTRA_DIST = rcdefines.h.in

CLEANFILES = rcdefines.h
DISTCLEANFILES = $(CLEANFILES)
