/* Make sure programs that override mmap don't mess us up. #290249 */

#include "headers.h"

/* A few basic stubs that do nothing. */
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	errno = ENOMEM;
	return MAP_FAILED;
}
int munmap(void *addr, size_t length)
{
	errno = ENOMEM;
	return -1;
}

int main(int argc, char *argv[])
{
	/* Don't loop forever. */
	alarm(10);

	/* Make sure we do an operation to trigger the sandbox. */
	open("/dev/null", 0);

	return 0;
}
