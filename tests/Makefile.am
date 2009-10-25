AT_FILES = $(wildcard *.at)
TESTSUITE = $(srcdir)/testsuite
DISTCLEANFILES = atconfig
EXTRA_DIST = atlocal.in package.m4.in $(AT_FILES) $(TESTSUITE) \
	test-skel-0.c \
	tests.h

AM_CPPFLAGS = -I$(top_srcdir)

check_PROGRAMS = \
	get-group \
	get-user \
	sb_true \
	sb_true_static \
	\
	access-0 \
	chmod-0 \
	chown-0 \
	creat-0 \
	creat64-0 \
	faccessat-0 \
	fchmodat-0 \
	fchownat-0 \
	fopen-0 \
	fopen64-0 \
	futimesat-0 \
	lchown-0 \
	link-0 \
	linkat-0 \
	linkat_static-0 \
	lutimes-0 \
	mkdir-0 \
	mkdir_static-0 \
	mkdirat-0 \
	mkfifo-0 \
	mkfifoat-0 \
	mknod-0 \
	mknodat-0 \
	open-0 \
	open_static-0 \
	open64-0 \
	openat-0 \
	openat_static-0 \
	openat64-0 \
	opendir-0 \
	rename-0 \
	renameat-0 \
	signal_static-0 \
	symlink-0 \
	symlinkat-0 \
	truncate-0 \
	truncate64-0 \
	unlink-0 \
	unlinkat-0 \
	utime-0 \
	utimensat-0 \
	utimensat_static-0 \
	utimes-0 \
	vfork-0 \
	\
	getcwd-gnulib_tst \
	libsigsegv_tst \
	sb_printf_tst

dist_check_SCRIPTS = \
	$(wildcard *-?.sh) \
	script-0 \
	trace-0

AM_LDFLAGS = `expr $@ : .*_static- >/dev/null && echo -all-static`

sb_printf_tst_CFLAGS = -I$(top_srcdir)/libsbutil -I$(top_srcdir)/libsbutil/include
sb_printf_tst_LDADD = $(top_builddir)/libsbutil/libsbutil.la

if HAVE_LIBSIGSEGV
libsigsegv_tst_LDADD = -lsigsegv
endif

check-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='src:tests' $(TESTSUITEFLAGS)

installcheck-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='src:tests:$(bindir)' $(TESTSUITEFLAGS)

clean-local:
	test ! -f '$(TESTSUITE)' || \
		$(SHELL) '$(TESTSUITE)' --clean

AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): $(AT_FILES)
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@
