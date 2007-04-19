AUTOMAKE_OPTIONS = foreign

INCLUDES = \
	-I$(top_srcdir)/include \
	$(RCSCRIPTS_DEFINES)

lib_LTLIBRARIES = librcutil.la

librcutil_la_LDFLAGS = \
	-Wl,--version-script,$(srcdir)/librcutil.map
librcutil_la_SOURCES = \
	debug.c        \
	string.c       \
	file.c         \
	config.c       \
	dynbuf.c       \
	simple-regex.c

EXTRA_DIST = librcutil.map
