EXTRA_DIST         = canonicalize.c symbols.in
CLEANFILES         = symbols.h

AM_CPPFLAGS        = -D_GNU_SOURCE -DPIC -fPIC -D_REENTRANT \
                     -DLIBSANDBOX_PATH=\"$(libdir)\" \
                     -DSANDBOX_BASHRC_PATH=\"$(pkgdatadir)\" \
		     -I$(top_srcdir)

lib_LTLIBRARIES         = libsandbox.la
libsandbox_la_SOURCES   = libsandbox.c $(top_srcdir)/localdecls.h
# Do not add -nostdlib or -nostartfiles, as then our constructor
# and destructor will not be executed ...
libsandbox_la_LDFLAGS   = -nodefaultlibs -lc -ldl
# We need -fexceptions here, else we do not catch exceptions
# (nptl/tst-cancelx4.c in glibc among others fails for wrapped functions).
libsandbox_la_CFLAGS    = -fexceptions

bin_PROGRAMS       = sandbox
sandbox_SOURCES    = sandbox.c sandbox.h sandbox_futils.c getcwd.c $(top_srcdir)/localdecls.h
sandbox_CFLAGS     = -DOUTSIDE_LIBSANDBOX

libsandbox.c: symbols.h

# Basically generates symbols.h from the function names in symbols.in.
# Not sure if we can count on the latest version the only symbol
# countaining '@@' ...
symbols.h: symbols.in
	@echo "Generating $@"; \
	if [ ! -e "$(LIBC_PATH)" ]; then\
		echo -e "\n*** Cannot find LIBC_PATH '$(LIBC_PATH)' !\n"; \
		exit 1; \
	fi; \
	echo "#ifndef __symbol_h" > $@; \
	echo -e "#define __symbol_h\n" >> $@; \
	for x in `cat $^`; do \
		sym=`readelf -s "$(LIBC_PATH)" 2>/dev/null | \
			awk '{ print $$8 }' | \
			grep "^$${x}@@" | \
			sort -u | \
			cut -d'@' -f3`; \
		echo "#define symname_$${x} \"$${x}\"" >> $@; \
		if [ -n "$${sym}" ]; then \
			echo "#define symver_$${x} \"$${sym}\"" >> $@; \
		else \
			echo "#define symver_$${x} NULL" >> $@; \
		fi; \
	done; \
	echo -e "\n#endif /* __symbol_h */" >> $@
