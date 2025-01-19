/* gnulib-specific glue logic that normally gnulib macros would set up.
 *
 * Copyright 1999-2015 Gentoo Foundation
 * Licensed under the GPL-2
 */

#pragma once

#define _GL_CONFIG_H_INCLUDED 1

#if defined __GNUC__ && defined __GNUC_MINOR__ && !defined __clang__
# define _GL_GNUC_PREREQ(major, minor) \
    ((major) < __GNUC__ + ((minor) <= __GNUC_MINOR__))
#elif defined __clang__
  /* clang really only groks GNU C 4.2.  */
# define _GL_GNUC_PREREQ(major, minor) \
    ((major) < 4 + ((minor) <= 2))
#else
# define _GL_GNUC_PREREQ(major, minor) 0
#endif

#ifndef _GL_ATTRIBUTE_DEALLOC
# if _GL_GNUC_PREREQ (11, 0)
#  define _GL_ATTRIBUTE_DEALLOC(f, i) __attribute__ ((__malloc__ (f, i)))
# else
#  define _GL_ATTRIBUTE_DEALLOC(f, i)
# endif
#endif

#define _GL_ATTRIBUTE_ALLOC_SIZE(args) __attribute__ ((__alloc_size__ args))
#define _GL_ATTRIBUTE_ALWAYS_INLINE __attribute__ ((__always_inline__))
#define _GL_ATTRIBUTE_CONST __attribute__ ((__const__))
#define _GL_ATTRIBUTE_COLD __attribute__ ((__cold__))
#define _GL_ATTRIBUTE_DEALLOC_FREE _GL_ATTRIBUTE_DEALLOC (free, 1)
#define _GL_ATTRIBUTE_DEPRECATED [[__deprecated__]]
#define _GL_ATTRIBUTE_FALLTHROUGH [[__fallthrough__]]
#define _GL_ATTRIBUTE_MALLOC __attribute__ ((__malloc__))
#define _GL_ATTRIBUTE_NODISCARD [[__nodiscard__]]
#define _GL_ATTRIBUTE_PURE __attribute__ ((__pure__))
#define _GL_ATTRIBUTE_RETURNS_NONNULL __attribute__ ((__returns_nonnull__))

#include "gl-inline.h"

/* musl does not have rawmemchr */
#if !HAVE_DECL_RAWMEMCHR
void *rawmemchr(const void *s, int c);
#endif

/* gnulib's rawmemchr uses static_assert */
#ifndef HAVE_RAWMEMCHR
#include <assert.h>
#endif

/* C23 compat */
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>

#define _GL_INTEGER_WIDTH(min, max) (((min) < 0) + _GL_COB128 (max))
#define _GL_COB128(n) (_GL_COB64 ((n) >> 31 >> 31 >> 2) + _GL_COB64 (n))
#define _GL_COB64(n) (_GL_COB32 ((n) >> 31 >> 1) + _GL_COB32 (n))
#define _GL_COB32(n) (_GL_COB16 ((n) >> 16) + _GL_COB16 (n))
#define _GL_COB16(n) (_GL_COB8 ((n) >> 8) + _GL_COB8 (n))
#define _GL_COB8(n) (_GL_COB4 ((n) >> 4) + _GL_COB4 (n))
#define _GL_COB4(n) (!!((n) & 8) + !!((n) & 4) + !!((n) & 2) + !!((n) & 1))

#ifndef UCHAR_WIDTH
# define UCHAR_WIDTH _GL_INTEGER_WIDTH (0, UCHAR_MAX)
#endif

#ifndef UINTPTR_WIDTH
# define UINTPTR_WIDTH _GL_INTEGER_WIDTH(0, UINTPTR_MAX)
#endif
