#include "headers.h"
#include "sbutil.h"

#define T(fmt, args...) \
	do { \
		printf(fmt "\n", ##args); \
		sb_printf(fmt "\n", ##args); \
	} while (0)

int main(int argc, char *argv[])
{
	T("%i", argc);
	T("%i", -argc);
	T("%d", 123);
	T("%d", -123);
	T("%u", 1000);
	T("%zi", argc);
	T("%zi", -argc);
	T("%zd", 123);
	T("%zd", -123);
	T("%zu", 1000);

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

	size_t i;
	for (i = 0; i < 6; ++i)
		T("%s%*s%s", "{pre}", i, "cow", "{post}");

	T("%%");

	void *moo = (void *)0x123456;
	T("%p", moo);

	return 0;
}
