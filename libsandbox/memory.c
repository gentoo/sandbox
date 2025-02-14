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

/* Ensure malloc returns aligned memory #565630 */
#define ALIGN_FACTOR 2
#define ALIGN_SIZE (ALIGN_FACTOR * sizeof(size_t))

void *malloc(size_t size)
{
	long pagesize = sysconf(_SC_PAGESIZE);

	if (__builtin_add_overflow(size, ALIGN_SIZE, &size) ||
			__builtin_add_overflow(size, pagesize - size % pagesize, &size)) {
		errno = ENOMEM;
		return NULL;
	}

	size_t *sp = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (sp == MAP_FAILED)
		return NULL;

	*sp = size;
	return sp + ALIGN_FACTOR;
}

void free(void *ptr)
{
	if (ptr == NULL)
		return;

	size_t *sp = ptr;
	sp -= ALIGN_FACTOR;

	if (munmap(sp, *sp))
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

	long pagesize = sysconf(_SC_PAGESIZE);

	if (__builtin_add_overflow(size, ALIGN_SIZE, &size) ||
			__builtin_add_overflow(size, pagesize - size % pagesize, &size)) {
		errno = ENOMEM;
		return NULL;
	}

	size_t *sp = ptr;
	sp -= 2;

	sp = mremap(sp, *sp, size, MREMAP_MAYMOVE);
	if (sp == MAP_FAILED)
		return NULL;

	*sp = size;
	return sp + ALIGN_FACTOR;
}

size_t malloc_size(void *ptr)
{
	if (!ptr)
		return 0;

	size_t *sp = ptr;
	sp -= 2;

	return *sp - ALIGN_SIZE;
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
