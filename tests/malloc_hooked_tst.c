/* Make sure programs that override malloc don't mess us up:
 *
 * libsandbox's __attribute__((constructor)) libsb_init ->
 *   libsandbox's malloc() ->
 *     dlsym("mmap") ->
 *       glibc's libdl calls malloc ->
 *         tcmalloc's internal code calls open ->
 *           libsandbox's open wrapper is hit ->
 *             libsandbox tries to initialize itself (since it never finished originally) ->
 *               libsandbox's malloc() ->
 *                 dlsym() -> deadlock
 * https://crbug.com/586444
 */

#include "headers.h"

static void *malloc_hook(size_t size, const void *caller)
{
	int urandom_fd = open("/dev/urandom", O_RDONLY);
	close(urandom_fd);
	return NULL;
}

void *(*__malloc_hook)(size_t, const void *) = &malloc_hook;

static void *thread_start(void *arg)
{
	return arg;
}

int main(int argc, char *argv[])
{
	/* Make sure we reference some pthread symbols, although we don't
	 * really want to execute it -- our malloc is limited. */
	if (argc < 0) {
		pthread_t tid;
		pthread_create(&tid, NULL, thread_start, NULL);
	}

	/* Trigger malloc! */
	if (malloc(100)) {}

	return 0;
}
