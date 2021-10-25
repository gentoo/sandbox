# Sandbox

Sandbox is a library (and helper utility) to run programs in a "sandboxed"
environment.  This is used as a QA measure to try and prevent applications from
modifying files they should not.

For example, in the Gentoo world we use it so we can build applications as root
and make sure that the build system does not do crazy things outside of its
build directory.  Such as install files to the live root file system or modify
config files on the fly.

For people who are familiar with the Debian "fakeroot" project or the RPM based
"InstallWatch", sandbox is in the same vein of projects.

## Method

The way sandbox works is that you prime a few environment variables (in order
to control the sandbox's behavior) and then stick it into the LD_PRELOAD
variable.  Then when the ELF loader runs, it will first load the sandbox
library.  Whenever an applications makes a library call that we have wrapped,
we'll check the arguments against the environment settings.  Based on that, any
access that is not permitted is logged and we return an error to the
application.  Any access that is permitted is of course forwarded along to the
real C library.

Static ELFs and setuid/setgid programs are executed with
[ptrace()](https://man7.org/linux/man-pages/man2/ptrace.2.html) instead.

## Availability

Sandbox supports multiple monitoring methods, but not all are available in all
system configurations.

### preload

The in-process LD_PRELOAD method should be available on any reasonable ELF-based
system as long as it uses dynamic linking.  Statically linked programs will run,
but will not be monitored, nor will set*id programs (because the C library will
clear LD_PRELOAD first).

Multiple ABIs are supported (e.g. x86 32-bit & 64-bit).

It has been tested & known to work with:
* Architecture
  * They all should work!
* Operating system
  * [Linux](https://kernel.org/) 2.4+
* C library
  * [GNU C library (glibc)](https://www.gnu.org/software/libc/) 2.2+
  * [uClibc](https://uclibc.org/) 0.9.26+
  * [musl](https://musl.libc.org/) 0.9.9+

### ptrace

The out-of-process ptrace method is available on Linux systems, works with
dynamic & static linking, and supports set*id programs (by forcing them to run
without any elevated privileges).

Multiple personalities are supported (e.g. PowerPC 32-bit & 64-bit).

NB: Does not work in userland emulators (e.g. QEMU) which do not provide ptrace
emulation.

It requires:
* Architecture
  * Alpha
  * ARM (32-bit EABI)
  * Blackfin
  * HPPA/PA-RISC (32-bit)
  * Itanium
  * PowerPC (32-bit & 64-bit)
  * s390 (32-bit & 64-bit)
  * SPARC (32-bit & 64-bit)
  * x86 (32-bit & 64-bit & x32)
* Operating system
  * [Linux](https://kernel.org/) 3.8+
* C library
  * They all should work!
