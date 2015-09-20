/*
 * sb_memory.c
 *
 * Simle debugging/logging macro's and functions.
 *
 * Copyright 1999-2008 Gentoo Foundation
 * Copyright 2004-2007 Martin Schlemmer <azarah@nosferatu.za.org>
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"

/* Make sure the C library doesn't rewrite calls to funcs libsandbox provides. */
#undef calloc
#undef malloc
#undef realloc
#undef strdup

void *
__xcalloc(size_t nmemb, size_t size, const char *file, const char *func, size_t line)
{
	void *ret = calloc(nmemb, size);

	if (ret == NULL)
		__sb_ebort(file, func, line, "calloc(%zu, %zu)\n", nmemb, size);

	return ret;
}

void *
__xmalloc(size_t size, const char *file, const char *func, size_t line)
{
	void *ret = malloc(size);

	if (ret == NULL)
		__sb_ebort(file, func, line, "malloc(%zu)\n", size);

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

	if (ret == NULL)
		__sb_ebort(file, func, line, "realloc(%p, %zu)\n", ptr, size);

	return ret;
}

char *
__xstrdup(const char *str, const char *file, const char *func, size_t line)
{
	char *ret = strdup(str);

	if (ret == NULL)
		__sb_ebort(file, func, line, "strdup(%p)\n", str);

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

	if (ret == NULL)
		__sb_ebort(file, func, line, "strndup(%p, %zu)\n", str, size);

	return ret;
}
