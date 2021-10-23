bin_PROGRAMS += %D%/sandbox

%C%_sandbox_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/libsbutil \
	-I$(top_srcdir)/libsbutil/include

%C%_sandbox_LDADD = libsbutil/libsbutil.la $(LIBDL)
%C%_sandbox_SOURCES = \
	%D%/environ.c \
	%D%/namespaces.c \
	%D%/options.c \
	%D%/sandbox.h \
	%D%/sandbox.c
