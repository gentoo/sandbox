AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libsandbox.la

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libsbutil \
	-I$(top_srcdir)/libsbutil/include \
	$(SANDBOX_DEFINES)

libsandbox_la_CFLAGS = $(CFLAG_EXCEPTIONS)
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
	-no-undefined \
	-nodefaultlibs \
	-avoid-version \
	$(LDFLAG_VER),libsandbox.map
libsandbox_la_SOURCES = \
	eqawarn.c    \
	libsandbox.h \
	libsandbox.c \
	lock.c       \
	memory.c     \
	trace.c      \
	wrappers.h   \
	wrappers.c   \
	canonicalize.c

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/libsandbox.la

libsandbox.c: libsandbox.map sb_nr.h
trace.c: trace_syscalls.h sb_nr.h $(TRACE_FILES)
wrappers.c: symbols.h

TRACE_FILES = $(wildcard $(srcdir)/trace/*.[ch] $(srcdir)/trace/*/*.[ch])

SCRIPT_DIR = $(top_srcdir)/scripts

SYMBOLS_FILE = $(srcdir)/symbols.h.in
SYMBOLS_LIST = $(shell $(SED) -n '/^[^\#]/p' $(SYMBOLS_FILE))
SYMBOLS_WRAPPERS = $(wildcard $(srcdir)/wrapper-funcs/*.[ch])
GEN_VERSION_MAP_SCRIPT = $(SCRIPT_DIR)/gen_symbol_version_map.awk
GEN_HEADER_SCRIPT = $(SCRIPT_DIR)/gen_symbol_header.awk
GEN_TRACE_SCRIPT = $(SCRIPT_DIR)/gen_trace_header.awk
SB_AWK = LC_ALL=C $(AWK) -v SYMBOLS_LIST="$(SYMBOLS_LIST)" -v srcdir="$(srcdir)" -f

libsandbox.map: $(SYMBOLS_FILE) $(GEN_VERSION_MAP_SCRIPT)
	$(AM_V_GEN)$(READELF) -s $(LIBC_PATH) | $(SB_AWK) $(GEN_VERSION_MAP_SCRIPT) > $@

symbols.h: $(SYMBOLS_FILE) $(GEN_HEADER_SCRIPT)
	$(AM_V_GEN)$(READELF) -s $(LIBC_PATH) | $(SB_AWK) $(GEN_HEADER_SCRIPT) > $@

SB_NR_FILE = $(srcdir)/sb_nr.h.in
sb_nr.h: symbols.h $(SB_NR_FILE)
	$(AM_V_GEN)$(EGREP) -h '^\#define SB_' $^ > $@

TRACE_MAKE_HEADER = \
	$(SB_AWK) $(GEN_TRACE_SCRIPT) -v MODE=gen | \
		$(COMPILE) -E -include $(top_srcdir)/headers.h - $$f | \
		$(SB_AWK) $(GEN_TRACE_SCRIPT) -v syscall_prefix=$$t > $$header
trace_syscalls.h: $(GEN_TRACE_SCRIPT)
if SB_SCHIZO
	$(AM_V_GEN)for pers in $(SB_SCHIZO_SETTINGS) ; do \
		t=_$${pers%:*}; \
		f=$${pers#*:}; \
		header=trace_syscalls$${t}.h; \
		$(TRACE_MAKE_HEADER) || exit $$?; \
	done
	@touch $@
else
	$(AM_V_GEN)t= f= header=$@; $(TRACE_MAKE_HEADER)
endif

EXTRA_DIST = $(SYMBOLS_FILE) $(SYMBOLS_WRAPPERS) $(SB_NR_FILE) $(TRACE_FILES)

CLEANFILES = libsandbox.map sb_nr.h symbols.h trace_syscalls*.h
DISTCLEANFILES = $(CLEANFILES)
