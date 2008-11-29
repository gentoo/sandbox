AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libsandbox.la

AM_CPPFLAGS = \
	$(SANDBOX_DEFINES) \
	-DPIC -fPIC

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libsbutil \
	-I$(top_srcdir)/libsbutil/include

# We need -fexceptions here, else we do not catch exceptions
# (nptl/tst-cancelx4.c in glibc among others fails for wrapped functions).
libsandbox_la_CFLAGS = -fexceptions
# Could use the following to libsandbox_la_LIBADD, but then libtool links it
# with --whole-archive, and libsandbox.so increase with a few KB in size:
#	$(top_builddir)/libsbutil/libsbutil.la
libsandbox_la_LIBSBLIB = $(top_builddir)/libsbutil/.libs/libsbutil.a
libsandbox_la_LIBADD = \
	-lc $(LIBDL) \
	$(libsandbox_la_LIBSBLIB)
# Do not add -nostdlib or -nostartfiles, as then our constructor
# and destructor will not be executed ...
libsandbox_la_LDFLAGS = \
	-nodefaultlibs \
	-Wl,--version-script,libsandbox.map
libsandbox_la_SOURCES = \
	libsandbox.h \
	libsandbox.c \
	memory.c     \
	wrappers.h   \
	wrappers.c   \
	canonicalize.c

libsandbox.c: libsandbox.map
wrappers.c: symbols.h

SYMBOLS_FILE = $(srcdir)/symbols.h.in
SYMBOLS_LIST = $(shell $(SED) -n '/^[^\#]/p' $(SYMBOLS_FILE))
SYMBOLS_WRAPPERS = $(wildcard  $(srcdir)/wrapper-funcs/*.[ch])
GEN_VERSION_MAP_SCRIPT = $(top_srcdir)/scripts/gen_symbol_version_map.awk
GEN_HEADER_SCRIPT = $(top_srcdir)/scripts/gen_symbol_header.awk

libsandbox.map: $(SYMBOLS_FILE) $(GEN_VERSION_MAP_SCRIPT)
	$(READELF) -h -s $(LIBC_PATH) | \
		$(AWK) -v SYMBOLS_LIST="$(SYMBOLS_LIST)" -f $(GEN_VERSION_MAP_SCRIPT) > $@

symbols.h: $(SYMBOLS_FILE) $(SYMBOLS_WRAPPERS) $(GEN_HEADER_SCRIPT)
	$(READELF) -h -s $(LIBC_PATH) | \
		$(AWK) -v SYMBOLS_LIST="$(SYMBOLS_LIST)" -f $(GEN_HEADER_SCRIPT) > $@

EXTRA_DIST = $(SYMBOLS_FILE) $(SYMBOLS_WRAPPERS)

CLEANFILES = libsandbox.map symbols.h
DISTCLEANFILES = $(CLEANFILES)
