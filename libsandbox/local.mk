lib_LTLIBRARIES += %D%/libsandbox.la

%C%_libsandbox_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I%D% \
	-I$(top_srcdir)/%D% \
	-I$(top_srcdir)/libsbutil \
	-I$(top_srcdir)/libsbutil/include

%C%_libsandbox_la_CFLAGS = $(CFLAG_EXCEPTIONS)
# Could use the following to libsandbox_la_LIBADD, but then libtool links it
# with --whole-archive, and libsandbox.so increase with a few KB in size:
#	libsbutil/libsbutil.la
libsbutil/.libs/libsbutil.a: libsbutil/libsbutil.la
%C%_libsandbox_la_LIBSBLIB = libsbutil/.libs/libsbutil.a
%C%_libsandbox_la_LIBADD = \
	-lc $(LIBDL) \
	$(%C%_libsandbox_la_LIBSBLIB)
# Do not add -nostdlib or -nostartfiles, as then our constructor
# and destructor will not be executed ...
%C%_libsandbox_la_LDFLAGS = \
	-no-undefined \
	-avoid-version \
	$(LDFLAG_VER),%D%/libsandbox.map
%C%_libsandbox_la_SOURCES = \
	%D%/libsandbox.h \
	%D%/libsandbox.c \
	%D%/lock.c       \
	%D%/memory.c     \
	%D%/trace.c      \
	%D%/wrappers.h   \
	%D%/wrappers.c   \
	%D%/canonicalize.c

install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/libsandbox.la
# Since we removed the .la file, libtool uninstall doesn't work,
# so we have to manually uninstall libsandbox.so ourselves.
uninstall-hook:
	rm -f $(DESTDIR)$(libdir)/libsandbox.so

%D%/libsandbox.c: %D%/libsandbox.map %D%/sb_nr.h
%D%/trace.c: %D%/trace_syscalls.h %D%/sb_nr.h $(TRACE_FILES)
%D%/wrappers.c: %D%/symbols.h

TRACE_FILES = $(wildcard $(top_srcdir)/%D%/trace/*.[ch] $(top_srcdir)/%D%/trace/*/*.[ch])

SCRIPT_DIR = $(top_srcdir)/scripts

SYMBOLS_FILE = $(top_srcdir)/%D%/symbols.h.in
SYMBOLS_WRAPPERS = $(wildcard $(top_srcdir)/%D%/wrapper-funcs/*.[ch])
GEN_VERSION_MAP_SCRIPT = $(SCRIPT_DIR)/gen_symbol_version_map.awk
GEN_HEADER_SCRIPT = $(SCRIPT_DIR)/gen_symbol_header.awk
GEN_TRACE_SCRIPT = $(SCRIPT_DIR)/gen_trace_header.awk
SB_AWK = LC_ALL=C $(AWK) -v SYMBOLS_FILE="$(SYMBOLS_FILE)" -v srcdir="$(top_srcdir)/%D%" -f

%D%/libsandbox.map: $(SYMBOLS_FILE) $(GEN_VERSION_MAP_SCRIPT)
	@$(MKDIR_P) %D%
	$(AM_V_GEN)$(READELF) -sW $(LIBC_PATH) | $(SB_AWK) $(GEN_VERSION_MAP_SCRIPT) > $@

%D%/symbols.h: $(SYMBOLS_FILE) $(GEN_HEADER_SCRIPT)
	@$(MKDIR_P) %D%
	$(AM_V_GEN)$(READELF) -sW $(LIBC_PATH) | $(SB_AWK) $(GEN_HEADER_SCRIPT) > $@

SB_NR_FILE = %D%/sb_nr.h.in
%D%/sb_nr.h: %D%/symbols.h $(SB_NR_FILE)
	@$(MKDIR_P) %D%
	$(AM_V_GEN)$(EGREP) -h '^\#define SB_' $^ > $@

TRACE_MAKE_HEADER = \
	$(SB_AWK) $(GEN_TRACE_SCRIPT) -v MODE=gen | \
		$(COMPILE) -E -P -include $(top_srcdir)/headers.h - $$f | \
		$(SB_AWK) $(GEN_TRACE_SCRIPT) -v syscall_prefix=$$t > $$header
%D%/trace_syscalls.h: $(SYMBOLS_FILE) $(GEN_TRACE_SCRIPT) $(SB_SCHIZO_HEADERS)
	@$(MKDIR_P) %D%
if SB_SCHIZO
	$(AM_V_GEN)touch $@
else
	$(AM_V_GEN)t= f= header=$@; $(TRACE_MAKE_HEADER)
endif

$(SB_SCHIZO_HEADERS): $(SYMBOLS_FILE) $(GEN_TRACE_SCRIPT)
	@$(MKDIR_P) %D%
	$(AM_V_GEN)for pers in $(SB_SCHIZO_SETTINGS) ; do \
		t=_$${pers%:*}; \
		f=$${pers#*:}; \
		header="%D%/trace_syscalls$${t}.h"; \
		if [ "$$header" = "$@" ]; then \
			$(TRACE_MAKE_HEADER) || exit $$?; \
			break; \
		fi; \
	done

EXTRA_DIST += $(SYMBOLS_FILE) $(SYMBOLS_WRAPPERS) $(SB_NR_FILE) $(TRACE_FILES)

CLEANFILES += \
	%D%/libsandbox.map \
	%D%/sb_nr.h \
	%D%/symbols.h \
	%D%/trace_syscalls*.h
