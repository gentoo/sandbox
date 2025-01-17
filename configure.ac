AC_PREREQ([2.71])
AC_INIT([sandbox], [2.42], [sandbox@gentoo.org])
AM_INIT_AUTOMAKE([1.15 dist-xz foreign no-dist-gzip silent-rules subdir-objects -Wall])
AM_SILENT_RULES([yes]) # AM_INIT_AUTOMAKE([silent-rules]) is broken atm
AC_CONFIG_HEADERS([sandboxconfig.h])
AC_CONFIG_MACRO_DIR([m4])

AC_MSG_CHECKING([environment state])
env 1>&AS_MESSAGE_LOG_FD
AC_MSG_RESULT([ok])

dnl Check for compiler and features first.
AC_PROG_CC
AM_PROG_CC_C_O
AC_USE_SYSTEM_EXTENSIONS

dnl Checks for programs.
AM_PROG_AR
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_AWK
AC_PROG_EGREP
AC_CHECK_TOOLS([READELF], [readelf eu-readelf], [false])
AM_MISSING_PROG([AUTOM4TE], [autom4te])

LT_INIT([disable-static])

AC_PREFIX_DEFAULT([/usr])

dnl multiple personality support (x86 & x86_64: multilib)
AC_MSG_CHECKING([for multiple personalities])
AC_ARG_ENABLE([personalities],
	[AS_HELP_STRING([--enable-personalities],[Support multiple Linux personalities using ptrace])],
	[],[enable_personalities="auto"])
AC_MSG_RESULT([$enable_personalities])
SB_PERSONALITIES_SETTINGS=
AC_DEFUN([SB_CHECK_PERSONALITIES],[dnl
		AC_MSG_CHECKING([checking for $1/$2 compiler support])
		ac_save_CFLAGS=$CFLAGS
		CFLAGS="$CFLAGS $2"
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
			#include <stdio.h>
		], [
			return 0;
		])], [
			enable_personalities=yes
			AS_VAR_APPEND([SB_PERSONALITIES_SETTINGS], " $1:$2")
			AS_VAR_APPEND([SB_PERSONALITIES_HEADERS], " libsandbox/trace_syscalls_$1.h")
			AC_MSG_RESULT([yes])
			AC_DEFINE_UNQUOTED([SB_PERSONALITIES_$1], 1, [Support for $1/$2 is available])
		], [
			AC_MSG_RESULT([no])
		])
		CFLAGS=$ac_save_CFLAGS
])
if test "x$enable_personalities" != "xno" ; then
	enable_personalities=no
	case $host in
	i686*linux*|\
	x86_64*linux*)
		SB_CHECK_PERSONALITIES([x86_64], [-m64])
		SB_CHECK_PERSONALITIES([x86], [-m32])
		SB_CHECK_PERSONALITIES([x32], [-mx32])
		;;
	s390*linux*)
		SB_CHECK_PERSONALITIES([s390x], [-m64])
		SB_CHECK_PERSONALITIES([s390], [-m31])
		;;
	sparc*linux*)
		SB_CHECK_PERSONALITIES([sparc64], [-m64])
		SB_CHECK_PERSONALITIES([sparc], [-m32])
		;;
	esac
	SB_PERSONALITIES_SETTINGS=${SB_PERSONALITIES_SETTINGS# }
	if test "x$enable_personalities" != "xno" ; then
		AC_DEFINE_UNQUOTED([SB_PERSONALITIES], ["$SB_PERSONALITIES_SETTINGS"], [Enable multiple personalities support])
	fi
fi
AC_SUBST(SB_PERSONALITIES_SETTINGS)
AC_SUBST(SB_PERSONALITIES_HEADERS)
AM_CONDITIONAL([SB_PERSONALITIES], [test "x$enable_personalities" != "xno"])

dnl this test fills up the stack and then triggers a segfault ...
dnl but it's hard to wrap things without a stack, so let's ignore
dnl this test for now ...
ac_cv_lib_sigsegv_stackoverflow_install_handler=false

dnl Checks for libraries.
dnl Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_PROG_EGREP

AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS_ONCE(m4_flatten([
	dirent.h
	dlfcn.h
	elf.h
	errno.h
	execinfo.h
	fcntl.h
	getopt.h
	grp.h
	inttypes.h
	libgen.h
	limits.h
	memory.h
	pthread.h
	pwd.h
	sched.h
	siginfo.h
	signal.h
	sigsegv.h
	stdarg.h
	stdbool.h
	stddef.h
	stdint.h
	stdio.h
	stdlib.h
	string.h
	strings.h
	syscall.h
	unistd.h
	utime.h
	sys/file.h
	sys/ioctl.h
	sys/mman.h
	sys/mount.h
	sys/param.h
	sys/prctl.h
	sys/ptrace.h
	sys/reg.h
	sys/socket.h
	sys/stat.h
	sys/syscall.h
	sys/time.h
	sys/types.h
	sys/uio.h
	sys/user.h
	sys/wait.h
	sys/xattr.h
	asm/ptrace.h
	linux/ptrace.h
]))

dnl Checks for typedefs, structures, and compiler characteristics.
dnl Do this after all headers have been checked.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_CHECK_DECLS_ONCE([environ])
AC_CHECK_TYPES([ptrdiff_t])
AC_CHECK_TYPES([sighandler_t, sig_t, __sighandler_t],,,[#include <signal.h>])

save_CPPFLAGS=$CPPFLAGS
CPPFLAGS="-I$srcdir $CPPFLAGS"
AC_CHECK_TYPES([struct ptrace_syscall_info],,,[#include "headers.h"])
AC_CHECK_TYPES([struct user_regs_struct, struct pt_regs],,,[#include "headers.h"])
AC_CHECK_SIZEOF([struct user_regs_struct],,[#include "headers.h"])
AC_CHECK_SIZEOF([struct pt_regs],,[#include "headers.h"])
CPPFLAGS=$save_CPPFLAGS

dnl Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_STAT
AC_CHECK_FUNCS_ONCE(m4_flatten([
	backtrace
	clone
	__clone2
	creat64
	faccessat
	fchmodat
	fchownat
	fopen64
	ftruncate
	futimesat
	getcwd
	lchown
	linkat
	lremovexattr
	lsetxattr
	lutimes
	memmove
	memcpy
	memset
	mkdir
	mkdirat
	mkdtemp
	mkfifoat
	mknodat
	mkostemp
	mkostemp64
	mkostemps
	mkostemps64
	mkstemp
	mkstemp64
	mkstemps
	mkstemps64
	mmap64
	open64
	openat
	openat64
	pathconf
	prctl
	process_vm_readv
	ptrace
	realpath
	remove
	removexattr
	renameat
	renameat2
	rmdir
	setenv
	setxattr
	strcasecmp
	strchr
	strdup
	strerror
	strndup
	strrchr
	strspn
	strstr
	strtok_r
	symlinkat
	truncate64
	unlinkat
	unshare
	utime
	utimensat
	__utimensat64
	__utimensat_time64
	utimes
]))
AC_CHECK_LIB([sigsegv], [stackoverflow_install_handler],
	[HAVE_LIBSIGSEGV=true],
	[HAVE_LIBSIGSEGV=false])
AM_CONDITIONAL(HAVE_LIBSIGSEGV, $HAVE_LIBSIGSEGV)

dnl Check for gcc atomic primitives
AC_MSG_CHECKING([for __sync_lock_test_and_set])
sltas=no
AC_LINK_IFELSE([AC_LANG_PROGRAM([],
	[int i; __sync_lock_test_and_set(&i, 1); return i;])],
	[sltas=yes]
)
if test "$sltas" = yes ; then
	AC_DEFINE([HAVE___SYNC_LOCK_TEST_AND_SET], [1], [GCC provides __sync_lock_test_and_set])
fi
AC_MSG_RESULT($sltas)

dnl Check if gcc provides va_copy or __va_copy (for librcutil)
AC_MSG_CHECKING([for va_copy])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
    #include <stdarg.h>
  ], [
    va_list ap, aq;
    va_copy(ap, aq);
  ])],
  [va_copy="va_copy"],
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
      #include <stdarg.h>
    ], [
      va_list ap, aq;
      __va_copy(ap, aq);
    ])],
    [va_copy="__va_copy"],
    [AC_MSG_ERROR([Unable to determine name of va_copy macro])]
  )]
)
AC_MSG_RESULT([$va_copy])
if test x"$va_copy" != xva_copy ; then
  AC_DEFINE_UNQUOTED([va_copy], [$va_copy],
    [Define to name of va_copy macro proviced by gcc if its not 'va_copy'.]
  )
fi

dnl Avoid footguns.
if test x"$enable_static" != xno ; then
  AC_MSG_ERROR([Building a static libsandbox.a is not supported])
fi
if test x"$enable_shared" != xyes ; then
  AC_MSG_ERROR([Omitting a libsandbox.so is not supported])
fi
if echo " $CFLAGS " | $EGREP ' -static ' >/dev/null 2>&1; then
  AC_MSG_ERROR([Using -static in CFLAGS is not supported])
fi
if echo " $LDFLAGS " | $EGREP ' -static ' >/dev/null 2>&1; then
  AC_MSG_ERROR([Using -static in LDFLAGS is not supported])
fi

dnl Some libc's like those on bsd have dlopen() in libc, and not libdl
AC_SEARCH_LIBS([dlopen], [dl])

dnl uClibc doesn't currently provide dlvsym() so lets
dnl verify the toolchain supports it
AC_CHECK_FUNCS([dlvsym])

dnl Check if libc provides RTLD_NEXT
AC_MSG_CHECKING([for RTLD_NEXT])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([
    #include <dlfcn.h>
  ], [
    #if !defined(RTLD_NEXT)
    # error no RTLD_NEXT
    #endif
  ])],
  [have_rtld_next="yes"],
  [have_rtld_next="no"]
)
AC_MSG_RESULT([$have_rtld_next])
if test x"$have_rtld_next" = xyes ; then
  AC_DEFINE([HAVE_RTLD_NEXT], [1], [Have RTLD_NEXT enabled libc])
fi

dnl we need to handle symbols differently based upon their version,
dnl but we have to know which symbols the libc supports first
AC_MSG_CHECKING([libc path])
echo "int main(void) { return 0; }" > libctest.c
try_libc_path="$CC $CFLAGS $CPPFLAGS $LDFLAGS -o libctest libctest.c > libctest.log 2>&1"
try_link() {
	(
		echo "try_link: trying: ${try_libc_path} $1"
		eval ${try_libc_path} $1
		ret=$?
		cat libctest.log
		if test ${ret} -eq 0 ; then
			echo "try_link: SUCCESS"
		else
			echo "try_link: FAILURE (ret = ${ret})"
		fi
		exit ${ret}
	) 1>&AS_MESSAGE_LOG_FD
}
LIBC_PATH=$(AS_IF(
	dnl GNU linkers (bfd, gold), LLVM lld, mold - searching for latest entry of:
	dnl     "/usr/lib/gcc/x86_64-pc-linux-gnu/8.3.0/../../../../lib64/libc.so"
	dnl     "/lib64/libc.so.6"
	dnl Note that mold prefixes output with "trace: " whereas others do not.
	[try_link -Wl,--trace],
		[$EGREP -o '/[[^ ]]*/libc.so.*' libctest.log | tail -n1],
	dnl Solaris linker
	[try_link -Wl,-m],
		[set -- `$EGREP -o '/[[^ ]]*/libc.so' libctest.log`; echo $1]
))
rm -f libctest*
if test x"$LIBC_PATH" = x || ! test -r "$LIBC_PATH" ; then
  AC_MSG_ERROR([Unable to determine LIBC PATH ($LIBC_PATH)])
fi
AC_DEFINE_UNQUOTED([LIBC_PATH], ["$LIBC_PATH"], [Full path to the libc])
AC_MSG_RESULT([$LIBC_PATH])
AC_SUBST([LIBC_PATH])

dnl when intercepting libc calls, we have to know the name of the
dnl libc to load and search with dl*() calls
AC_MSG_CHECKING([libc version])
dnl the sed script at the end here looks funny but it's ok ...
echo "int main(void) { return 0; }" > libctest.c
$CC $CFLAGS $CPPFLAGS $LDFLAGS -o libctest libctest.c
LIBC_VERSION=$(
	$READELF -d libctest | \
	$EGREP 'NEEDED.* \@<:@libc\..*so' | \
	$AWK '{print $NF}' | [sed -e 's:\[::' -e 's:\]::']
)
rm -f libctest*
if test "$LIBC_VERSION"x = x ; then
  AC_MSG_ERROR([Unable to determine LIBC VERSION])
fi
AC_MSG_RESULT([$LIBC_VERSION])
AC_DEFINE_UNQUOTED([LIBC_VERSION], ["$LIBC_VERSION"], [Name of libc to hook into])

dnl Check for /proc/ features
AC_DEFUN([SB_CHECK_PATH],[dnl
	AC_MSG_CHECKING([for $1])
	if test -e "$1$2" ; then
		AC_DEFINE([SANDBOX]m4_translit(m4_toupper([$1]), [/$], [_d]),
		          [1], [System has $1])
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
	fi
])
SB_CHECK_PATH([/proc/self/fd], [/0])
SB_CHECK_PATH([/dev/fd], [/0])
SB_CHECK_PATH([/proc/self/cmdline])
SB_CHECK_PATH([/proc/1/cmdline])
SB_CHECK_PATH([/proc/$$/cmdline])

dnl We add to CPPFLAGS rather than doing AC_DEFINE_UNQUOTED
dnl so we dont have to worry about fully expanding all of
dnl the variables ($sysconfdir defaults to "$prefix/etc")
SANDBOX_DEFINES='-DETCDIR="\"$(sysconfdir)\"" -DLIBSANDBOX_PATH="\"$(libdir)\"" -DSANDBOX_BASHRC_PATH="\"$(pkgdatadir)\""'
AC_SUBST([SANDBOX_DEFINES])

dnl Check for toolchain features
dnl We need -fexceptions here, else we do not catch exceptions
dnl (nptl/tst-cancelx4.c in glibc among others fails for wrapped functions)
AC_MSG_CHECKING([how to enable exception handling])
for CFLAG_EXCEPTIONS in -fexceptions "" ; do
	ac_save_CFLAGS=$CFLAGS
	CFLAGS="$CFLAGS -fexceptions"
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [])],[ok=yes],[ok=no])
	CFLAGS=$ac_save_CFLAGS
	test "$ok" = "yes" && break
done
AC_MSG_RESULT($CFLAG_EXCEPTIONS)
AC_SUBST([CFLAG_EXCEPTIONS])

CPPFLAGS="$CPPFLAGS -D_REENTRANT"
AX_CFLAGS_WARN_ALL
AC_DEFUN([SB_CHECK_CFLAG],[AX_CHECK_COMPILE_FLAG([$1],[CFLAGS="$CFLAGS $1"])])
SB_CHECK_CFLAG([-fdata-sections])
SB_CHECK_CFLAG([-ffunction-sections])
dnl We have our own wrapped memory functions.
SB_CHECK_CFLAG([-fno-builtin-calloc])
SB_CHECK_CFLAG([-fno-builtin-malloc])
SB_CHECK_CFLAG([-fno-builtin-realloc])
AC_DEFUN([SB_CHECK_LDFLAG],[AX_CHECK_LINK_FLAG([-Wl,$1],[LDFLAGS="$LDFLAGS -Wl,$1"])])
SB_CHECK_LDFLAG([--as-needed])
dnl Itanium systems do not like this flag, so disable it to avoid warnings.
if test "$host_cpu" != "ia64" ; then
	SB_CHECK_LDFLAG([--gc-sections])
fi
case $host_os in
	dnl bsd is dumb (environ)
	*linux*) SB_CHECK_LDFLAG([--no-undefined]) ;;
esac

AC_DEFUN([SB_CHECK_LDFLAG_VER],[dnl
if test "x${LDFLAG_VER}" = "x" ; then
	echo '{};' > conftest.map
	AX_CHECK_LINK_FLAG([-Wl,$1,conftest.map],[LDFLAG_VER="-Wl,$1"])
	rm -f conftest.map
fi])
LDFLAG_VER=
SB_CHECK_LDFLAG_VER([--version-script])
SB_CHECK_LDFLAG_VER([-M])
if test "x${LDFLAG_VER}" = "x" ; then
	AC_MSG_ERROR([unable to find a linker flag for versioning])
fi
AC_SUBST([LDFLAG_VER])

AC_CONFIG_TESTDIR([tests])

AC_CONFIG_FILES([src/sandbox.sh], [chmod +x src/sandbox.sh])
AC_CONFIG_FILES([
	Makefile
	etc/sandbox.d/00default
	tests/atlocal
	tests/package.m4
])
AC_OUTPUT
