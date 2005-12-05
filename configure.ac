AC_PREREQ([2.59])
AC_INIT([sandbox], [1.2.17], [sandbox@gentoo.org])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([config.h])

dnl Checks for programs.
AC_PROG_CC
AC_ISC_POSIX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_AWK
AC_CHECK_PROGS([READELF], [readelf], [false])

AC_ENABLE_SHARED
AC_DISABLE_STATIC
dnl Next four lines is a hack to prevent libtool checking for CXX/F77
m4_undefine([AC_PROG_CXX])
m4_defun([AC_PROG_CXX],[])
m4_undefine([AC_PROG_F77])
m4_defun([AC_PROG_F77],[])
AC_PROG_LIBTOOL

AC_PREFIX_DEFAULT([/usr])

dnl Checks for libraries.
dnl Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([ \
	fcntl.h limits.h memory.h stddef.h \
	stdlib.h string.h strings.h sys/file.h \
	sys/param.h sys/time.h unistd.h utime.h \
])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

dnl Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([ \
	bzero ftruncate getcwd lchown memmove \
	mempcpy memset mkdir pathconf realpath \
	rmdir setenv strcasecmp strchr strdup \
	strerror strndup strrchr strspn strstr \
])

dnl Some libc's like those on bsd have dlopen() in libc, and not libdl
AC_CHECK_LIB([dl], [dlopen],
	[have_libdl="yes"],
	[have_libdl="no"]
)
if test x"$have_libdl" = xyes ; then
	LIBDL="-ldl"
	AC_SUBST([LIBDL])
	DL_LIB="dl"
else
	DL_LIB="c"
	AC_CHECK_LIB([c], [dlopen],
		[],
		[AC_MSG_ERROR([Unable to determine library providing dlopen])]
	)
fi

dnl uClibc doesn't currently provide dlvsym() so lets
dnl verify the toolchain supports it
AC_CHECK_LIB([$DL_LIB], [dlvsym],
	[AC_DEFINE([HAVE_DLVSYM], [1], [libdl supports dlvsym])],
	[AC_DEFINE([HAVE_DLVSYM], [0], [libdl does not support dlvsym])]
)

dnl when using libc5, (f)trucate's offset argument type is size_t with
dnl libc5, but it's off_t with libc6 (glibc2).
AC_MSG_CHECKING([truncate argument type])
TRUNC_ARG_TYPE=`echo '#include <unistd.h>' | $CC -E - | grep -q 'truncate.*size_t'`
if test "$TRUNC_ARG_TYPE"x != x ; then
	AC_MSG_RESULT([size_t])
	AC_DEFINE([TRUNCATE_T], [size_t], [truncate arg type])
else
	AC_MSG_RESULT([off_t])
	AC_DEFINE([TRUNCATE_T], [off_t], [truncate arg type])
fi

dnl Check if libc provides RTLD_NEXT
AC_MSG_CHECKING([for RTLD_NEXT])
AC_TRY_COMPILE([
#define _GNU_SOURCE
#include <dlfcn.h>
], [
#if !defined(RTLD_NEXT)
# error no RTLD_NEXT
#endif

int
main (void)
{
  return 0;
}
],
	[have_rtld_next="yes"],
	[have_rtld_next="no"]
)
AC_MSG_RESULT([$have_rtld_next])
AM_CONDITIONAL([HAVE_RTLD_NEXT], [test x"$have_rtld_next" = xyes])

dnl we need to handle symbols differently based upon their version, 
dnl but we have to know which symbols the libc supports first
AC_MSG_CHECKING([libc path])
echo "int main(void) { return 0; }" > libctest.c
$CC -Wall -o libctest libctest.c
LIBC_PATH=`$CC $CFLAGS -Wl,-verbose -o libctest libctest.c 2>&1 | \
	$AWK '/attempt to open/ { if (($4 ~ /\/libc\.so/) && ($5 == "succeeded")) LIBC = $4; }; END {print LIBC}'`
rm -f libctest.c
if test "$LIBC_PATH"x = x || ! test -r "$LIBC_PATH"; then
	AC_MSG_ERROR([Unable to determine LIBC PATH])
fi
AC_MSG_RESULT([$LIBC_PATH])
AC_SUBST([LIBC_PATH])

dnl when intercepting libc calls, we have to know the name of the 
dnl libc to load and search with dl*() calls
AC_MSG_CHECKING([libc version])
dnl the sed script at the end here looks funny but it's ok ...
dnl they're m4 escape sequences for left and right brackets
LIBC_VERSION=`readelf -d libctest | grep NEEDED.*libc\\.so | $AWK '{print $NF}' | sed -e 's:\@<:@::' -e 's:\@:>@::'`
if test "$LIBC_VERSION"x = x ; then
	AC_MSG_ERROR([Unable to determine LIBC VERSION])
fi
rm -f libctest
AC_MSG_RESULT([$LIBC_VERSION])
AC_DEFINE_UNQUOTED([LIBC_VERSION], ["$LIBC_VERSION"], [name of libc to hook into])

dnl check if we have 32bit or 64bit output
AC_ARG_ENABLE([multilib],
	AS_HELP_STRING([--enable-multilib],
		[enable building for multilib setups (default=disabled)]),
	[enable_multilib="$enableval"],
	[enable_multilib="no"]
)

if test x"$enable_multilib" != xno ; then
	AC_DEFINE_UNQUOTED([SB_HAVE_MULTILIB], [1], [have multilib enabled system])
fi

AC_OUTPUT([
	Makefile
	scripts/Makefile
	data/Makefile
	src/Makefile
])
