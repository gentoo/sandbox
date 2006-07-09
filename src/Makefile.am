AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = sandbox

AM_CPPFLAGS = \
	$(SANDBOX_DEFINES)       \
	-DPIC -fPIC -D_REENTRANT \
	-I$(top_srcdir) -Wall

INCLUDES = \
	-I$(top_srcdir)/libsbutil         \
	-I$(top_srcdir)/libsbutil/include

sandbox_CFLAGS = -DOUTSIDE_LIBSANDBOX
sandbox_LDADD = $(top_builddir)/libsbutil/libsbutil.la
sandbox_SOURCES = sandbox.c

