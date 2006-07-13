AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libsandbox.la

AM_CPPFLAGS = \
	$(SANDBOX_DEFINES)       \
	-DPIC -fPIC -D_REENTRANT \
	-I$(top_srcdir) -Wall

INCLUDES = \
	-I$(top_srcdir)/libsbutil         \
	-I$(top_srcdir)/libsbutil/include

# We need -fexceptions here, else we do not catch exceptions
# (nptl/tst-cancelx4.c in glibc among others fails for wrapped functions).
libsandbox_la_CFLAGS = -fexceptions
# Could use the following to libsandbox_la_LIBADD, but then libtool links it
# with --whole-archive:
#	$(top_builddir)/libsbutil/libsbutil.la
libsandbox_la_LIBSBOBJS = $(wildcard  $(top_builddir)/libsbutil/*.lo)
libsandbox_la_LIBADD = \
	-lc $(LIBDL) \
	$(libsandbox_la_LIBSBOBJS)
# Do not add -nostdlib or -nostartfiles, as then our constructor
# and destructor will not be executed ...
libsandbox_la_LDFLAGS = \
	-nodefaultlibs \
	-Wl,--version-script,libsandbox.map
libsandbox_la_SOURCES = \
	libsandbox.h \
	libsandbox.c \
	wrappers.h   \
	wrappers.c   \
	canonicalize.c

libsandbox.c: libsandbox.map symbols.h

SYMBOLS_FILE = $(srcdir)/symbols.h.in
SYMBOLS_LIST = $(shell $(EGREP) -v '^\#' $(SYMBOLS_FILE))
SYMBOLS_WRAPPERS = $(wildcard  $(srcdir)/wrapper-funcs/*.c)
GEN_VERSION_MAP_SCRIPT = $(top_srcdir)/scripts/gen_symbol_version_map.awk
GEN_HEADER_SCRIPT = $(top_srcdir)/scripts/gen_symbol_header.awk

libsandbox.map: $(SYMBOLS_FILE) $(GEN_VERSION_MAP_SCRIPT)
	@echo "Generating $@"; \
	if [ ! -e "$(LIBC_PATH)" ]; then\
		echo -e "\n*** Cannot find LIBC_PATH '$(LIBC_PATH)' !\n"; \
		exit 1; \
	fi; \
	export SYMBOLS="$(SYMBOLS_LIST)"; \
	if ! $(READELF) -h -s $(LIBC_PATH) | \
		$(AWK) -f $(GEN_VERSION_MAP_SCRIPT) > $@ ; \
	then \
		echo "\n*** Failed to generate '$@' !\n"; \
		exit 1; \
	fi

symbols.h: $(SYMBOLS_FILE) $(SYMBOLS_WRAPPERS) $(GEN_HEADER_SCRIPT)
	@echo "Generating $@"; \
	if [ ! -e "$(LIBC_PATH)" ]; then\
		echo -e "\n*** Cannot find LIBC_PATH '$(LIBC_PATH)' !\n"; \
		exit 1; \
	fi; \
	export SYMBOLS="$(SYMBOLS_LIST)"; \
	if ! $(READELF) -h -s $(LIBC_PATH) | \
		$(AWK) -f $(GEN_HEADER_SCRIPT) > $@ ; \
	then \
		echo "\n*** Failed to generate '$@' !\n"; \
		exit 1; \
	fi

EXTRA_DIST = $(SYMBOLS_FILE) $(SYMBOLS_WRAPPERS)

CLEANFILES = libsandbox.map symbols.h
DISTCLEANFILES = $(CLEANFILES)
