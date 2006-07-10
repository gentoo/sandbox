AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
	$(SANDBOX_DEFINES)       \
	-DPIC -fPIC -D_REENTRANT \
	-I$(top_srcdir) -Wall

INCLUDES = -I$(srcdir)/include
LOCAL_INCLUDES = $(top_srcdir)/localdecls.h

noinst_LTLIBRARIES = libsbutil.la

libsbutil_la_SOURCES =                        \
	sbutil.h                              \
	sbutil.c                              \
	sbio.c                                \
	include/rcscripts/rctypes.h           \
	include/rcscripts/rcutil.h            \
	include/rcscripts/util/list.h         \
	include/rcscripts/util/str_list.h     \
	include/rcscripts/util/simple-regex.h \
	include/rcscripts/util/debug.h        \
	src/debug.c                           \
	include/rcscripts/util/string.h       \
	src/string.c                          \
	include/rcscripts/util/file.h         \
	src/file.c                            \
	include/rcscripts/util/config.h       \
	src/config.c                          \
	include/rcscripts/util/dynbuf.h       \
	src/dynbuf.c                          \
	$(LOCAL_INCLUDES)

