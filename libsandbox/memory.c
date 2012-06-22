/* memory.c
 * Minimal mmap-based malloc/free implementation to be used by libsandbox
 * internal routines, since we can't trust the current process to have a
 * malloc/free implementation that is sane and available at all times.
 *
 * Note that we want to check and return NULL as normal and not call other
 * x*() type funcs.  That way the higher levels (which are calling the x*()
 * versions) will see NULL and trigger the right kind of error message.
 *
 * Copyright 1999-2009 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "libsandbox.h"
#include "sbutil.h"

/* Well screw me sideways, someone decided to override mmap() #290249
 * We probably don't need to include the exact sym version ...
 */
static void *(*_sb_mmap)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
static void *sb_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	if (!_sb_mmap)
		_sb_mmap = get_dlsym("mmap", NULL);
	return _sb_mmap(addr, length, prot, flags, fd, offset);
}
#define mmap sb_mmap
static int (*_sb_munmap)(void *addr, size_t length);
static int sb_munmap(void *addr, size_t length)
{
	if (!_sb_munmap)
		_sb_munmap = get_dlsym("munmap", NULL);
	return _sb_munmap(addr, length);
}
#define munmap sb_munmap

#define SB_MALLOC_TO_MMAP(ptr) ((void*)(((size_t*)ptr) - 1))
#define SB_MMAP_TO_MALLOC(ptr) ((void*)(((size_t*)ptr) + 1))
#define SB_MALLOC_TO_SIZE(ptr) (*((size_t*)SB_MALLOC_TO_MMAP(ptr)))

void *malloc(size_t size)
{
	size_t *ret;
	size += sizeof(size_t);
	ret = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (ret == MAP_FAILED)
		return NULL;
	*ret = size;
	return SB_MMAP_TO_MALLOC(ret);
}

void free(void *ptr)
{
	if (ptr == NULL)
		return;
	if (munmap(SB_MALLOC_TO_MMAP(ptr), SB_MALLOC_TO_SIZE(ptr)))
		sb_ebort("sandbox memory corruption with free(%p): %s\n",
			ptr, strerror(errno));
}

/* Hrm, implement a zalloc() ? */
void *calloc(size_t nmemb, size_t size)
{
	void *ret = malloc(nmemb * size); /* dont care about overflow */
	if (ret)
		memset(ret, 0, nmemb * size);
	return ret;
}

void *realloc(void *ptr, size_t size)
{
	void *ret;
	size_t old_malloc_size;

	if (ptr == NULL)
		return malloc(size);
	if (size == 0) {
		free(ptr);
		return ptr;
	}

	old_malloc_size = SB_MALLOC_TO_SIZE(ptr);
	/* Since mmap() is heavy, don't bother shrinking */
	if (size <= old_malloc_size)
		return ptr;
	ret = malloc(size);
	if (!ret)
		return ret;
	memcpy(ret, ptr, MIN(size, old_malloc_size));
	free(ptr);
	return ret;
}

char *strdup(const char *s)
{
	size_t len;
	char *ret;

	if (s == NULL)
		return NULL;

	len = strlen(s);
	ret = malloc(len + 1);
	if (!ret)
		return ret;
	return memcpy(ret, s, len + 1);
}
