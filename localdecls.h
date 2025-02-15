/*
 * File: localdecls.h
 *
 * Copyright 1999-2004 Gentoo Foundation
 * Distributed under the terms of the GNU General Public License v2
 */

#ifndef __LOCALDECLS_H__
#define __LOCALDECLS_H__

/* take care of broken ld loading */
#if defined(__GLIBC__) && !defined(__UCLIBC__)

# if __GLIBC__ <= 2 && __GLIBC_MINOR__ <= 2
#  define BROKEN_RTLD_NEXT
#  define LIBC 5
# endif

# if !defined(BROKEN_RTLD_NEXT)
#  if defined(__mips__)
#   define BROKEN_RTLD_NEXT
#  endif
# endif

#else

#if 0
# if defined(__FreeBSD__)
#  define BROKEN_RTLD_NEXT
# endif
#endif

#endif

#ifdef PATH_MAX
# define SB_PATH_MAX PATH_MAX * 2
# if (SB_PATH_MAX >= INT_MAX) || (SB_PATH_MAX < PATH_MAX)
#  undef SB_PATH_MAX
#  define SB_PATH_MAX PATH_MAX + 25
#  if (SB_PATH_MAX >= INT_MAX) || (SB_PATH_MAX < PATH_MAX)
#   error SB_PATH_MAX too big!
#  endif
# endif
#else
# error PATH_MAX not defined!
#endif

#ifndef MAP_ANONYMOUS
# define MAP_ANONYMOUS MAP_ANON
#endif

#ifndef O_CLOEXEC
# define O_CLOEXEC 0
#endif
#ifndef O_TMPFILE
# define O_TMPFILE 0
#endif

#if !HAVE_DECL_ENVIRON
extern char **environ;
#endif

#ifndef HAVE_SIGHANDLER_T
# if defined(HAVE_SIG_T)
typedef sig_t sighandler_t;
# elif defined(HAVE___SIGHANDLER_T)
typedef __sighandler_t sighandler_t;
# else
#  error "your system blows chunks: no signal() type"
# endif
#endif

#define HAVE_TRACE_REGS
#if defined(HAVE_STRUCT_PT_REGS) && defined(HAVE_STRUCT_USER_REGS_STRUCT)
/*
 * Some systems have broken kernel headers who declare struct pt_regs as
 * smaller than what the kernel actually operates on.  If we have both
 * reg structs available, pick the one that is larger in the hopes that
 * we won't randomly clobber memory.
 */
# if SIZEOF_STRUCT_PT_REGS < SIZEOF_STRUCT_USER_REGS_STRUCT
#  undef HAVE_STRUCT_PT_REGS
# endif
#endif
#if defined(HAVE_STRUCT_PT_REGS)
typedef struct pt_regs trace_regs;
#elif defined(HAVE_STRUCT_USER_REGS_STRUCT)
typedef struct user_regs_struct trace_regs;
#else
# undef HAVE_TRACE_REGS
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#endif
#ifndef MAX
#define MAX(x, y) ((x) < (y) ? (y) : (x))
#endif
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#define ALIGN_DOWN(base, size)     ((base) & -(size))
#define ALIGN_UP(base, size)       ALIGN_DOWN((base) + (size) - 1, (size))
#define PTR_ALIGN_DOWN(base, size) ((__typeof__(base))ALIGN_DOWN((uintptr_t)(base), (size)))
#define PTR_ALIGN_UP(base, size)   ((__typeof__(base))ALIGN_UP((uintptr_t)(base), (size)))

/* If the system is old and does not support *at funcs, then define
 * it ourself.  Shouldn't matter based on how we use it.
 */
#ifndef AT_FDCWD
# define AT_FDCWD -100
# define AT_SYMLINK_NOFOLLOW 0
#endif

#ifdef HAVE_DLVSYM
/* Taken from glibc */
# define symbol_version(_real, _name, _version) \
	__asm__ (".symver " #_real "," #_name "@" #_version)
# define default_symbol_version(_real, _name, _version) \
	__asm__ (".symver " #_real "," #_name "@@" #_version)
#else
# define dlvsym(_lib, _sym, _ver) dlsym(_lib, _sym)
#endif

#ifndef HAVE_STRLCPY
static size_t strlcpy(char *dst, const char *src, size_t size)
{
	size_t srclen = strlen(src);
	size_t copylen = (srclen < size ? srclen : size - 1);
	memcpy(dst, src, copylen);
	dst[copylen] = '\0';
	return srclen;
}
#endif

/* Taken from glibc */
# define strong_alias(_name, _aliasname) \
	extern __typeof (_name) _aliasname __attribute__ ((alias (#_name)));
# define weak_alias(_name, _aliasname) \
	extern __typeof (_name) _aliasname __attribute__ ((weak, alias (#_name)));

#define attribute_hidden __attribute__((visibility("hidden")))

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define __noreturn __attribute__((noreturn))
#define __printf(x, y) __attribute__((__format__(__printf__, x, y)))

#endif
