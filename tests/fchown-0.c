/*
 * https://bugs.gentoo.org/599706
 *
 */

#include "headers.h"

int main(int argc, char *argv[])
{
	if (argc < 3)
		return -2;

	uid_t uid = atoi(argv[1]);
	gid_t gid = atoi(argv[2]);
	/* The sandbox catches this:
	 *
	 *   int fd = open(argv[3], O_RDWR);
	 *
	 * And it /should/ catch this:
	 *
	 *    int fd = open(argv[3], O_RDONLY);
	 *
	 * ...but the latter only works when /proc/self/fd/%i
	 * is available.
	 */
#ifdef SANDBOX_PROC_SELF_FD
	int fd = open(argv[3], O_RDONLY);
#else
	int fd = open(argv[3], O_RDWR);
#endif
	int fchown_result = fchown(fd, uid, gid);
	close(fd);
	return fchown_result;
}
