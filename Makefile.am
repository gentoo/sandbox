bin_PROGRAMS		= sandbox
lib_LTLIBRARIES 	= libsandbox.la
datadir			= @datadir@
AM_CFLAGS		= -D_GNU_SOURCE -DPIC -fPIC -D_REENTRANT

libsandbox_la_SOURCES 	= libsandbox.c sandbox_futils.c localdecls.h
libsandbox_la_LDFLAGS 	= -Wc,-nostdlib -lc -ldl
libsandbox_la_CFLAGS	= -Wc,-nostdlib -D_REENTRANT -D_GNU_SOURCE \
	-DSANDBOX_BASHRC_PATH=\"$(pkgdatadir)\" \
	-DLIBSANDBOX_PATH=\"$(libdir)\"


sandbox_SOURCES		= sandbox.c sandbox.h sandbox_futils.c getcwd.c
sandbox_CFLAGS		= -DSANDBOX_BASHRC_PATH=\"$(pkgdatadir)\"	\
			-DLIBSANDBOX_PATH=\"$(libdir)\"

BUILT_SOURCES		= localdecls.h

dist_pkgdata_DATA	= sandbox.bashrc

localdecls.h:	Makefile
	./create-localdecls

AUTOMAKE_OPTIONS	= dist-bzip2 no-dist-gzip
EXTRA_DIST		= sandbox.bashrc canonicalize.c create-localdecls libctest.c

libctest.c:
canonicalize.c:
