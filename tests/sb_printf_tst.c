#include "headers.h"
#include "sbutil.h"

#define _T(func, fmt, args...) func("%i:[%s] " fmt "\n", __LINE__, fmt, ##args)
#define T(fmt, args...) \
	do { \
		_T(printf, fmt, ## args); \
		_T(sb_printf, fmt, ## args); \
	} while (0)

int main(int argc, char *argv[])
{
	/* sandbox outputs to stderr, so unify it */
	dup2(STDOUT_FILENO, STDERR_FILENO);
	setbuf(stdout, NULL);

	T("%i", argc);
	T("%i", -argc);
	T("%d", 123);
	T("%d", -123);
	T("%u", 1000);
	T("%zi", (ssize_t)argc);
	T("%zi", (ssize_t)-argc);
	T("%zd", (ssize_t)123);
	T("%zd", (ssize_t)-123);
	T("%zu", (size_t)1000);

	T("%x", argc);
	T("%x", 0xabcdef);
	T("%X", argc);
	T("%X", 0xabcdef);

	T("%c", 'a');
	T("%c", '0');
	T("%c", 'K');
	T("%s", "wOOf");
	T("%s", "CoW");
	T("%s", "!HI!");

	int i;
	for (i = 0; i < 6; ++i)
		T("%s%*s%s", "{pre}", i, "cow", "{post}");

	T("%%");

	void *moo = (void *)0x123456;
	T("%p", moo);

	return 0;
}
