/*
 * Make sure the process_vm_readv func can work with strings with different
 * alignments and lengths.
 *
 * https://bugs.gentoo.org/560396
 */

/* We want to make some bad calls. */
#undef _FORTIFY_SOURCES

#include "tests.h"

/* Make sure the buffer spans multiple pages. */
#define SIZE 0x1000
/* Make sure the buffer has plenty of slack space before/after. */
static char buf[SIZE * 8];

/* The smaller the span, the # of calls goes up: O(N*N*2+N). */
#define COUNT 0x20
#define STRIDE (SIZE / COUNT)

/* Some hacks to defeat gcc warnings so we can use bad pointers. */
volatile uintptr_t offset = 0;
#define non_const_ptr(ptr) ((void *)((uintptr_t)(ptr) + offset))

#define check_ptr(addr) \
({ \
	printf("  open(%p)\n", addr); \
	ret = open64(non_const_ptr(addr), O_RDONLY); \
	assert(ret == -1 && errno == EFAULT); \
})

int main(int argc, char *argv[])
{
	int ret;
	char *path = PTR_ALIGN_UP((char *)buf, SIZE);
	size_t start, end;

	setbuf(stdout, NULL);

	printf("some bad pointers\n");
	check_ptr(NULL);
	check_ptr((void *)-1);

	printf("lots of good pointers\n");
	printf("  buf = %p\n", buf);
	printf("  path = %p\n", path);

	for (start = 0; start < SIZE * 2 + STRIDE; start += STRIDE) {
		char *p = path + start;
		for (end = start + STRIDE; end < SIZE * 2 + STRIDE; end += STRIDE) {
			size_t len = end - start;
			printf("  open(%p -> %p [+%#zx])\n", p, p + len, len);
			memset(p, 'a', len);
			path[end] = '\0';
			ret = open64(p, O_RDONLY);
			assert(ret == -1 && (errno == ENOENT || errno == ENAMETOOLONG));
		}
	}

	return 0;
}
