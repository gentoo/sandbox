AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libsandbox.la
bin_PROGRAMS = sandbox

AM_CPPFLAGS = \
	-DPIC -fPIC -D_REENTRANT  \
	-DLIBSANDBOX_PATH=\"$(libdir)\"         \
	-DSANDBOX_BASHRC_PATH=\"$(pkgdatadir)\" \
	-I$(top_srcdir) -Wall

LOCAL_INCLUDES = $(top_srcdir)/localdecls.h

# We need -fexceptions here, else we do not catch exceptions
# (nptl/tst-cancelx4.c in glibc among others fails for wrapped functions).
libsandbox_la_CFLAGS = -fexceptions
if HAVE_RTLD_NEXT
libsandbox_la_CFLAGS += -DHAVE_RTLD_NEXT
endif
# Do not add -nostdlib or -nostartfiles, as then our constructor
# and destructor will not be executed ...
libsandbox_la_LIBADD = -lc $(LIBDL)
libsandbox_la_LDFLAGS = \
	-nodefaultlibs \
	-Wl,--version-script,libsandbox.map
libsandbox_la_SOURCES = \
	libsandbox.c      \
	libsandbox.h      \
	getcwd.c          \
	canonicalize.c    \
	sandbox_utils.c   \
	$(LOCAL_INCLUDES)

sandbox_CFLAGS = -DOUTSIDE_LIBSANDBOX
sandbox_SOURCES = \
	sandbox.c         \
	sandbox.h         \
	sandbox_utils.c   \
	libsandbox.h      \
	getcwd.c          \
	$(LOCAL_INCLUDES)

libsandbox.c: libsandbox.map symbols.h

SYMBOLS_FILE = $(srcdir)/symbols.h.in
SYMBOLS_LIST = $(shell $(EGREP) -v '^\#' $(SYMBOLS_FILE))
GEN_VERSION_MAP_SCRIPT = $(top_srcdir)/scripts/gen_symbol_version_map.awk
GEN_HEADER_SCRIPT = $(top_srcdir)/scripts/gen_symbol_header.awk

libsandbox.map: $(SYMBOLS_FILE) $(GEN_VERSION_MAP_SCRIPT)
	@echo "Generating $@"; \
	if [ ! -e "$(LIBC_PATH)" ]; then\
		echo -e "\n*** Cannot find LIBC_PATH '$(LIBC_PATH)' !\n"; \
		exit 1; \
	fi; \
	export SYMBOLS="$(SYMBOLS_LIST)"; \
	if ! $(READELF) -s $(LIBC_PATH) | \
		$(AWK) -f $(GEN_VERSION_MAP_SCRIPT) > $@ ; \
	then \
		echo "\n*** Failed to generate '$@' !\n"; \
		exit 1; \
	fi

symbols.h: $(SYMBOLS_FILE) $(GEN_HEADER_SCRIPT)
	@echo "Generating $@"; \
	if [ ! -e "$(LIBC_PATH)" ]; then\
		echo -e "\n*** Cannot find LIBC_PATH '$(LIBC_PATH)' !\n"; \
		exit 1; \
	fi; \
	export SYMBOLS="$(SYMBOLS_LIST)"; \
	if ! $(READELF) -s $(LIBC_PATH) | \
		$(AWK) -f $(GEN_HEADER_SCRIPT) > $@ ; \
	then \
		echo "\n*** Failed to generate '$@' !\n"; \
		exit 1; \
	fi

EXTRA_DIST = $(SYMBOLS_FILE)

CLEANFILES = libsandbox.map symbols.h
DISTCLEANFILES = $(CLEANFILES)
