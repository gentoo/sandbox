AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(srcdir)/include \
	$(SANDBOX_DEFINES)

LOCAL_INCLUDES = $(top_srcdir)/localdecls.h

noinst_LTLIBRARIES = libsbutil.la

libsbutil_la_LDFLAGS = -no-undefined
libsbutil_la_SOURCES =                        \
	sbutil.h                              \
	get_sandbox_conf.c                    \
	get_sandbox_confd.c                   \
	get_sandbox_lib.c                     \
	get_sandbox_rc.c                      \
	get_sandbox_log.c                     \
	get_tmp_dir.c                         \
	environment.c                         \
	sb_backtrace.c                        \
	sb_efuncs.c                           \
	sb_gdb.c                              \
	sb_method.c                           \
	sb_open.c                             \
	sb_read.c                             \
	sb_write.c                            \
	sb_write_fd.c                         \
	sb_close.c                            \
	sb_printf.c                           \
	sb_proc.c                             \
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
	gnulib/areadlink.h                    \
	gnulib/areadlink-with-size.c          \
	gnulib/bitrotate.c                    \
	gnulib/bitrotate.h                    \
	gnulib/canonicalize.c                 \
	gnulib/canonicalize.h                 \
	gnulib/careadlinkat.h                 \
	gnulib/dosname.h                      \
	gnulib/file-set.c                     \
	gnulib/file-set.h                     \
	gnulib/gl-inline.h                    \
	gnulib/glue.h                         \
	gnulib/hash.c                         \
	gnulib/hash.h                         \
	gnulib/hash-pjw.c                     \
	gnulib/hash-pjw.h                     \
	gnulib/hash-triple.c                  \
	gnulib/hash-triple.h                  \
	gnulib/pathmax.h                      \
	gnulib/same-inode.h                   \
	gnulib/xalloc.h                       \
	gnulib/xalloc-oversized.h             \
	gnulib/xgetcwd.h                      \
	$(LOCAL_INCLUDES)

EXTRA_DIST = headers.h
