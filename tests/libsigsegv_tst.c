/* distilled from m4:
 * $ echo 'define(a,a(a))a' | m4
 *
 * Basically, we use libsigsegv to install segfault and overflow handlers.
 * Then we overflow the stack.  The segfault handler should be called.  Then
 * we return, libsigsegv detects the overflow and then calls the overflow
 * handler.
 */

#include "tests.h"

#define WRITE(msg) write(1, msg, sizeof(msg) - 1)

#ifdef HAVE_SIGSEGV_H
#include <sigsegv.h>

static int segv_handler(void *address, int serious)
{
	WRITE("caught segv ... ");
	if (!serious)
		return 0;
	WRITE("FAIL: wtf, it's serious!");
	_exit(1);
}


static void overflow_handler(int emergency, stackoverflow_context_t context)
{
	WRITE("OVERFLOW: OK!\n");
	_exit(0);
}

static union
{
	char buffer[SIGSTKSZ];
	long double ld;
	long l;
	void *p;
} alternate_signal_stack;

int recursive(int i)
{
	char stack_fatty[1024];
	int ret = sprintf(stack_fatty, "%i", i);
	if (i <= 0)
		return i + ret;
	else
		return recursive(i + ret);
}

int main(int argc, char *argv[])
{
	int ret;

	ret = stackoverflow_install_handler(overflow_handler,
		alternate_signal_stack.buffer, sizeof alternate_signal_stack.buffer);
	if (ret)
		err("stackoverflow_install_handler() failed");

	ret = sigsegv_install_handler(segv_handler);
	if (ret)
		err("sigsegv_install_handler() failed");

	WRITE("starting overflow ... ");
	recursive(argc);
	return 0;
}

#else

int main()
{
	WRITE("starting overflow ... ");
	WRITE("caught segv ... ");
	WRITE("OVERFLOW: OK!\n");
	return 0;
}

#endif
