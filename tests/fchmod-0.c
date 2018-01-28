/*
 * https://bugs.gentoo.org/599706
 *
 */

#include "headers.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
		return -2;

	int mode = 0;
	sscanf(argv[1], "%i", &mode);
	/* The sandbox catches this:
	 *
	 *   int fd = open(argv[2], O_RDWR);
	 *
	 * And it /should/ catch this:
	 *
	 *    int fd = open(argv[2], O_RDONLY);
	 *
	 * ...but the latter only works when /proc/self/fd/%i
	 * is available.
	 *
	 */
#ifdef SANDBOX_PROC_SELF_FD
	int fd = open(argv[2], O_RDONLY);
#else
	int fd = open(argv[2], O_RDWR);
#endif
	int fchmod_result = fchmod(fd, (mode_t)mode);
	close(fd);
	return fchmod_result;
}
