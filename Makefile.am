AUTOMAKE_OPTIONS	= dist-bzip2 no-dist-gzip
dist_pkgdata_DATA	= sandbox.bashrc
EXTRA_DIST			= sandbox.bashrc canonicalize.c libctest.c
AM_CFLAGS			= -D_GNU_SOURCE -DPIC -fPIC -D_REENTRANT -DLIBC_VERSION=\"$(LIBC_VERSION)\"

lib_LTLIBRARIES 		= libsandbox.la
libsandbox_la_SOURCES 	= libsandbox.c sandbox_futils.c localdecls.h
libsandbox_la_LDFLAGS 	= -Wc,-nostdlib -lc -ldl
libsandbox_la_CFLAGS	= -Wc,-nostdlib -D_REENTRANT -D_GNU_SOURCE \
	-DSANDBOX_BASHRC_PATH=\"$(pkgdatadir)\" \
	-DLIBSANDBOX_PATH=\"$(libdir)\"

bin_PROGRAMS		= sandbox
sandbox_SOURCES		= sandbox.c sandbox.h sandbox_futils.c getcwd.c
sandbox_CFLAGS		= -DSANDBOX_BASHRC_PATH=\"$(pkgdatadir)\" -DLIBSANDBOX_PATH=\"$(libdir)\" -DOUTSIDE_LIBSANDBOX
