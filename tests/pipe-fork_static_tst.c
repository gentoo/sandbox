/*
https://bugs.gentoo.org/364877
written by Victor Stinner <victor.stinner@haypocalc.com>
*/

#include "headers.h"

int main(int argc, char *argv[])
{
	const size_t n = 1024* 1024;
	char *data;
	alarm(10);
	data = malloc(n);
	memset(data, 'a', n);
	if (write(1, data, n)) {}
	free(data);
	return 0;
}
