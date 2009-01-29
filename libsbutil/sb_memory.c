/*
 * debug.c
 *
 * Simle debugging/logging macro's and functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

void *
__xcalloc(size_t nmemb, size_t size, const char *file, const char *func, size_t line)
{
	void *ret = calloc(nmemb, size);

	if (ret == NULL) {
		SB_EERROR("calloc()", " %s:%s():%zu: calloc(%zu, %zu) failed: %s\n",
			file, func, line, nmemb, size, strerror(errno));
		abort();
	}

	return ret;
}

void *
__xmalloc(size_t size, const char *file, const char *func, size_t line)
{
	void *ret = malloc(size);

	if (ret == NULL) {
		SB_EERROR("malloc()", " %s:%s():%zu: malloc(%zu) failed: %s\n",
			file, func, line, size, strerror(errno));
		abort();
	}

	return ret;
}

void *
__xzalloc(size_t size /*, const char *file, const char *func, size_t line */)
{
	return memset(xmalloc(size), 0x00, size);
}

void *
__xrealloc(void *ptr, size_t size, const char *file, const char *func, size_t line)
{
	void *ret = realloc(ptr, size);

	if (ret == NULL) {
		SB_EERROR("realloc()", " %s:%s():%zu: realloc(%p, %zu) failed: %s\n",
			file, func, line, ptr, size, strerror(errno));
		abort();
	}

	return ret;
}

char *
__xstrdup(const char *str, const char *file, const char *func, size_t line)
{
	char *ret = strdup(str);

	if (ret == NULL) {
		SB_EERROR("strdup()", " %s:%s():%zu: strdup(%p) failed: %s\n",
			file, func, line, str, strerror(errno));
		abort();
	}

	return ret;
}

#ifndef HAVE_STRNDUP
static inline char *sb_strndup(const char *str, size_t n)
{
	size_t r;
	for (r = 0; r < n; ++r)
		if (!str[r])
			break;

	char *ret = xmalloc(r + 1);
	memcpy(ret, str, r);
	ret[r] = '\0';
	return ret;
}
# define strndup sb_strndup
#endif

char *
__xstrndup(const char *str, size_t size, const char *file, const char *func, size_t line)
{
	char *ret = strndup(str, size);

	if (ret == NULL) {
		SB_EERROR("strndup()", " %s:%s():%zu: strndup(%p, %zu) failed: %s\n",
			file, func, line, str, size, strerror(errno));
		abort();
	}

	return ret;
}
