AUTOMAKE_OPTIONS = foreign

INCLUDES = -I$(srcdir)/include

noinst_LTLIBRARIES = librcutil.la

librcutil_la_SOURCES =                        \
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
	src/dynbuf.c

