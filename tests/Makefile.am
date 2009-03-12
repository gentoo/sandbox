AT_FILES = $(wildcard *.at)
TESTSUITE = $(srcdir)/testsuite
DISTCLEANFILES = atconfig
EXTRA_DIST = atlocal.in package.m4.in $(AT_FILES) $(TESTSUITE) \
	test-skel-0.c \
	tests.h

INCLUDES = -I$(top_srcdir)

check_PROGRAMS = \
	get-group \
	get-user \
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
	lutimes-0 \
	mkdir-0 \
	mkdirat-0 \
	mkfifo-0 \
	mkfifoat-0 \
	mknod-0 \
	mknodat-0 \
	open-0 \
	open64-0 \
	openat-0 \
	openat64-0 \
	opendir-0 \
	rename-0 \
	renameat-0 \
	symlink-0 \
	symlinkat-0 \
	truncate-0 \
	truncate64-0 \
	unlink-0 \
	unlinkat-0 \
	utime-0 \
	utimensat-0 \
	utimes-0 \
	\
	getcwd-gnulib_tst \
	libsigsegv_tst \
	sb_printf_tst

dist_check_SCRIPTS = \
	access-1.sh \
	chmod-1.sh \
	chown-1.sh \
	fchmodat-1.sh \
	fchownat-1.sh \
	fchownat-2.sh \
	lchown-1.sh \
	mkdir-1.sh \
	mkdir-2.sh \
	mkdir-3.sh \
	open-1.sh \
	script-0 \
	script-1.sh \
	script-2.sh \
	script-3.sh \
	script-4.sh \
	script-5.sh \
	script-6.sh

sb_printf_tst_CFLAGS = -I$(top_srcdir)/libsbutil -I$(top_srcdir)/libsbutil/include
sb_printf_tst_LDADD = $(top_builddir)/libsbutil/libsbutil.la

if HAVE_LIBSIGSEGV
libsigsegv_tst_LDADD = -lsigsegv
endif

check-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' $(TESTSUITEFLAGS)

installcheck-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) '$(TESTSUITE)' AUTOTEST_PATH='$(bindir)' $(TESTSUITEFLAGS)

clean-local:
	test ! -f '$(TESTSUITE)' || \
		$(SHELL) '$(TESTSUITE)' --clean

AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): $(AT_FILES)
	$(AUTOTEST) -I '$(srcdir)' -o $@.tmp $@.at
	mv $@.tmp $@
