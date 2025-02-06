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

/* Pick a value to guarantee alignment requirements. #565630 */
#define MIN_ALIGN (2 * sizeof(void *))

/* Well screw me sideways, someone decided to override mmap() #290249
 * We probably don't need to include the exact sym version ...
 */
static void *(*_sb_mmap)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
static void *sb_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	if (!_sb_mmap)
#ifdef HAVE_MMAP64
		_sb_mmap = get_dlsym("mmap64", NULL);
#else
		_sb_mmap = get_dlsym("mmap", NULL);
#endif
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

static void *(*_sb_mremap)(void *old_address, size_t old_size, size_t new_size, int flags);
static void *sb_mremap(void *old_address, size_t old_size, size_t new_size, int flags)
{
	if (!_sb_mremap)
		_sb_mremap = get_dlsym("mremap", NULL);
	return _sb_mremap(old_address, old_size, new_size, flags);
}
#define mremap sb_mremap

#define SB_MALLOC_TO_MMAP(ptr) ((void*)((uintptr_t)(ptr) - MIN_ALIGN))
#define SB_MMAP_TO_MALLOC(ptr) ((void*)((uintptr_t)(ptr) + MIN_ALIGN))
#define SB_MALLOC_TO_MMAP_SIZE(ptr) (*((size_t*)SB_MALLOC_TO_MMAP(ptr)))
#define SB_MALLOC_TO_SIZE(ptr) (SB_MALLOC_TO_MMAP_SIZE(ptr) - MIN_ALIGN)

void *malloc(size_t size)
{
	if (__builtin_add_overflow(size, MIN_ALIGN, &size)) {
		errno = ENOMEM;
		return NULL;
	}
	size_t *ret = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (ret == MAP_FAILED)
		return NULL;
	*ret = size;
	return SB_MMAP_TO_MALLOC(ret);
}

void free(void *ptr)
{
	if (ptr == NULL)
		return;
	if (munmap(SB_MALLOC_TO_MMAP(ptr), SB_MALLOC_TO_MMAP_SIZE(ptr)))
		sb_ebort("sandbox memory corruption with free(%p): %s\n",
			ptr, strerror(errno));
}

/* Hrm, implement a zalloc() ? */
void *calloc(size_t nmemb, size_t size)
{
	if (__builtin_mul_overflow(nmemb, size, &size)) {
		errno = ENOMEM;
		return NULL;
	}
	void *ret = malloc(size);
	if (ret)
		memset(ret, 0, size);
	return ret;
}

void *realloc(void *ptr, size_t size)
{
	if (ptr == NULL)
		return malloc(size);
	if (size == 0) {
		free(ptr);
		return NULL;
	}
	if (__builtin_add_overflow(size, MIN_ALIGN, &size)) {
		errno = ENOMEM;
		return NULL;
	}
	size_t *sp = SB_MALLOC_TO_MMAP(ptr);
	size_t old_size = *sp;
	sp = mremap(sp, old_size, size, MREMAP_MAYMOVE);
	if (sp == MAP_FAILED)
		return NULL;
	*sp = size;
	return SB_MMAP_TO_MALLOC(sp);
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
