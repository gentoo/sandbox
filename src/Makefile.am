AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = sandbox

AM_CPPFLAGS = \
	$(SANDBOX_DEFINES) \
	-DPIC -fPIC

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libsbutil \
	-I$(top_srcdir)/libsbutil/include

sandbox_LDADD = $(top_builddir)/libsbutil/libsbutil.la $(LIBDL)
sandbox_SOURCES = \
	environ.c \
	sandbox.h \
	sandbox.c

install-exec-hook:
	sed -i.tmp \
		's:__SANDBOX_TESTING:\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00:' \
		$(DESTDIR)$(bindir)/sandbox
	rm -f $(DESTDIR)$(bindir)/sandbox.tmp
