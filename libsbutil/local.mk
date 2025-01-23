noinst_LIBRARIES += %D%/libsbutil.a

%C%_libsbutil_a_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(SIXTY_FOUR_FLAGS) \
	-I$(top_srcdir)/%D% \
	-I$(top_srcdir)/%D%/include \
	-I$(top_srcdir)/%D%/gnulib
%C%_libsbutil_a_CFLAGS = -fPIC
%C%_libsbutil_a_SOURCES = \
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
	%D%/src/dynbuf.c
