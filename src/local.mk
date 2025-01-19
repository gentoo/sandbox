bin_PROGRAMS += %D%/sandbox

%C%_sandbox_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(SIXTY_FOUR_FLAGS) \
	-I$(top_srcdir)/libsbutil \
	-I$(top_srcdir)/libsbutil/include

%C%_sandbox_LDADD = libsbutil/libsbutil.a $(LIBDL)
%C%_sandbox_SOURCES = \
	%D%/environ.c \
	%D%/namespaces.c \
	%D%/options.c \
	%D%/sandbox.h \
	%D%/sandbox.c
