noinst_LTLIBRARIES += %D%/libsbutil.la

%C%_libsbutil_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(SIXTY_FOUR_FLAGS) \
	-I$(top_srcdir)/%D% \
	-I$(top_srcdir)/%D%/include
%C%_libsbutil_la_LDFLAGS = -no-undefined
%C%_libsbutil_la_SOURCES = \
	%D%/sbutil.h                              \
	%D%/get_sandbox_conf.c                    \
	%D%/get_sandbox_confd.c                   \
	%D%/get_sandbox_lib.c                     \
	%D%/get_sandbox_rc.c                      \
	%D%/get_sandbox_log.c                     \
	%D%/get_tmp_dir.c                         \
	%D%/environment.c                         \
	%D%/sb_backtrace.c                        \
	%D%/sb_efuncs.c                           \
	%D%/sb_gdb.c                              \
	%D%/sb_method.c                           \
	%D%/sb_open.c                             \
	%D%/sb_read.c                             \
	%D%/sb_stat.c                             \
	%D%/sb_write.c                            \
	%D%/sb_write_fd.c                         \
	%D%/sb_close.c                            \
	%D%/sb_printf.c                           \
	%D%/sb_proc.c                             \
	%D%/sb_memory.c                           \
	%D%/include/rcscripts/rcutil.h            \
	%D%/include/rcscripts/util/str_list.h     \
	%D%/include/rcscripts/util/debug.h        \
	%D%/src/debug.c                           \
	%D%/include/rcscripts/util/string.h       \
	%D%/src/string.c                          \
	%D%/include/rcscripts/util/file.h         \
	%D%/src/file.c                            \
	%D%/include/rcscripts/util/config.h       \
	%D%/src/config.c                          \
	%D%/include/rcscripts/util/dynbuf.h       \
	%D%/src/dynbuf.c                          \
	%D%/gnulib/areadlink.h                    \
	%D%/gnulib/areadlink-with-size.c          \
	%D%/gnulib/bitrotate.c                    \
	%D%/gnulib/bitrotate.h                    \
	%D%/gnulib/canonicalize.c                 \
	%D%/gnulib/canonicalize.h                 \
	%D%/gnulib/careadlinkat.h                 \
	%D%/gnulib/dosname.h                      \
	%D%/gnulib/file-set.c                     \
	%D%/gnulib/file-set.h                     \
	%D%/gnulib/gl-inline.h                    \
	%D%/gnulib/glue.h                         \
	%D%/gnulib/hash.c                         \
	%D%/gnulib/hash.h                         \
	%D%/gnulib/hash-pjw.c                     \
	%D%/gnulib/hash-pjw.h                     \
	%D%/gnulib/hash-triple.c                  \
	%D%/gnulib/hash-triple.h                  \
	%D%/gnulib/pathmax.h                      \
	%D%/gnulib/same-inode.h                   \
	%D%/gnulib/xalloc.h                       \
	%D%/gnulib/xalloc-oversized.h             \
	%D%/gnulib/xgetcwd.h
