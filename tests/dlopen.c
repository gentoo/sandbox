/* By Mike Frysinger <vapier@gentoo.org>; released into public domain */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

#define FAIL(fmt, args...) errx(1, fmt, ## args)

static void *xdlopen(char *lib, int flag)
{
	void *hdl;
	if (!strcmp(lib, "RTLD_DEFAULT"))
		hdl = RTLD_DEFAULT;
	else if (!strcmp(lib, "RTLD_NEXT"))
		hdl = RTLD_NEXT;
	else {
		hdl = dlopen(lib, flag);
		if (!hdl)
			FAIL("dlopen(\"%s\") failed: %s", lib, dlerror());
	}
	printf("dlopen(\"%s\") returned %p\n", lib, hdl);
	return hdl;
}

static int xdlclose(void *hdl)
{
	if (hdl != RTLD_DEFAULT && hdl != RTLD_NEXT) {
		if (dlclose(hdl))
			FAIL("dlcose() failed: %s", dlerror());
		printf("dlclose(%p) worked\n", hdl);
	} else
		printf("dlclose() not needed for RTLD_{DEFAULT,NEXT}\n");
	return 0;
}

static void *xdlsym(void *hdl, char *sym)
{
	void *sym_hdl = dlsym(hdl, sym);
	if (!sym_hdl)
		FAIL("dlsym(\"%s\") failed: %s", sym, dlerror());
	printf("dlsym(\"%s\") returned %p\n", sym, sym_hdl);
	return sym_hdl;
}

static void *xdlvsym(void *hdl, char *sym, char *ver)
{
	void *sym_hdl = dlvsym(hdl, sym, ver);
	if (!sym_hdl)
		FAIL("dlvsym(\"%s\", \"%s\") failed: %s", sym, ver, dlerror());
	printf("dlvsym(\"%s\", \"%s\") returned %p\n", sym, ver, sym_hdl);
	return sym_hdl;
}

static int do_dlopen(int argc, char *argv[])
{
	char *lib = argv[1];
	if (!lib)
		FAIL("Usage: dlopen <lib name|RTLD_DEFAULT|RTLD_NEXT>");

	void *hdl = xdlopen(lib, RTLD_LAZY);

	return xdlclose(hdl);
}

static int do_dlsym(int argc, char *argv[])
{
	if (argc != 3)
		FAIL("Usage: dlsym <lib name|RTLD_DEFAULT|RTLD_NEXT> <sym>");

	char *lib = argv[1];
	char *hdl = xdlopen(lib, RTLD_LAZY);

	char *sym = argv[2];
	/* void *sym_hdl = */ xdlsym(hdl, sym);

	return xdlclose(hdl);
}

static int do_dlvsym(int argc, char *argv[])
{
	if (argc != 4)
		FAIL("Usage: dlvsym <lib name|RTLD_DEFAULT|RTLD_NEXT> <sym> <version>");

	char *lib = argv[1];
	char *hdl = xdlopen(lib, RTLD_LAZY);

	char *sym = argv[2];
	char *ver = argv[3];
	/* void *sym_hdl = */ xdlvsym(hdl, sym, ver);

	return xdlclose(hdl);
}

int main(int argc, char *argv[])
{
	int ret;
	char *argv0 = strrchr(argv[0], '/');
	argv0 = argv0 ? argv0 + 1 : argv[0];
	if (!strcmp(argv0, "dlsym"))
		ret = do_dlsym(argc, argv);
	else if (!strcmp(argv0, "dlvsym"))
		ret = do_dlvsym(argc, argv);
	else
		ret = do_dlopen(argc, argv);
	printf("%1$s: all tests %1$sED\n", ret ? "FAIL" : "PASS");
	return ret;
}
