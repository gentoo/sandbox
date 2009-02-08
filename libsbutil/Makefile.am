AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
	$(SANDBOX_DEFINES) \
	-DPIC -fPIC

INCLUDES = \
	-I$(top_srcdir) \
	-I$(srcdir)/include
LOCAL_INCLUDES = $(top_srcdir)/localdecls.h

noinst_LTLIBRARIES = libsbutil.la

libsbutil_la_LDFLAGS = -no-undefined
libsbutil_la_SOURCES =                        \
	sbutil.h                              \
	get_sandbox_conf.c                    \
	get_sandbox_lib.c                     \
	get_sandbox_rc.c                      \
	get_sandbox_log.c                     \
	get_tmp_dir.c                         \
	is_env_on.c                           \
	is_env_off.c                          \
	sb_open.c                             \
	sb_read.c                             \
	sb_write.c                            \
	sb_close.c                            \
	sb_printf.c                           \
	sb_memory.c                           \
	include/rcscripts/rcutil.h            \
	include/rcscripts/util/str_list.h     \
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

