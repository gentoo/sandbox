bin_PROGRAMS		= sandbox
lib_LTLIBRARIES 	= libsandbox.la

AM_CFLAGS		= -D_GNU_SOURCE -DPIC -fPIC -D_REENTRANT

libsandbox_la_SOURCES 	= libsandbox.c sandbox_futils.c localdecls.h
libsandbox_la_LDFLAGS 	= -Wc,-nostdlib
libsandbox_la_CFLAGS	= -Wc,-nostdlib -D_REENTRANT -D_GNU_SOURCE

sandbox_SOURCES		= sandbox.c sandbox.h sandbox_futils.c getcwd.c

BUILT_SOURCES		= localdecls.h


localdecls.h:	Makefile
	./create-localdecls

AUTOMAKE_OPTIONS	= dist-bzip2
