/*
 * Initialize various namespaces
 *
 * Copyright 1999-2015 Gentoo Foundation
 * Licensed under the GPL-2
 */

#include "headers.h"
#include "sbutil.h"
#include "sandbox.h"

#ifdef __linux__

#include <net/if.h>

#ifndef HAVE_UNSHARE
# ifdef __NR_unshare
#  define unshare(x) syscall(__NR_unshare, x)
# else
#  define unshare(x) -1
# endif
#endif

#define xmount(...) sb_assert(mount(__VA_ARGS__) == 0)
#define xmkdir(...) sb_assert(mkdir(__VA_ARGS__) == 0)
#define xchmod(...) sb_assert(chmod(__VA_ARGS__) == 0)
#define xsymlink(...) sb_assert(symlink(__VA_ARGS__) == 0)

#define xasprintf(fmt, ...) \
({ \
	int _ret = asprintf(fmt, __VA_ARGS__); \
	if (_ret == 0) \
		sb_perr("asprintf(%s) failed", #fmt); \
	_ret; \
})
#define xfopen(path, ...) \
({ \
	FILE *_ret = fopen(path, __VA_ARGS__); \
	if (_ret == 0) \
		sb_perr("fopen(%s) failed", #path); \
	_ret; \
})

static void ns_user_switch(int uid, int gid, int nuid, int ngid)
{
#ifdef CLONE_NEWUSER
	FILE *fp;
	char *map;

	if (uid == nuid || unshare(CLONE_NEWUSER))
		return;

	fp = xfopen("/proc/self/uid_map", "we");
	xasprintf(&map, "%i %i 1", nuid, uid);
	fputs(map, fp);
	fclose(fp);
	free(map);

	fp = xfopen("/proc/self/setgroups", "we");
	fputs("deny", fp);
	fclose(fp);

	fp = xfopen("/proc/self/gid_map", "we");
	xasprintf(&map, "%i %i 1\n", ngid, gid);
	fputs(map, fp);
	fclose(fp);
	free(map);
#endif
}

static void ns_net_setup(void)
{
#ifdef CLONE_NEWNET
	if (unshare(CLONE_NEWNET))
		return;

	int sock = socket(AF_LOCAL, SOCK_DGRAM|SOCK_CLOEXEC, 0);
	struct ifreq ifr;

	strcpy(ifr.ifr_name, "lo");
	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
		sb_perr("ioctl(SIOCGIFFLAGS, lo) failed");
	strcpy(ifr.ifr_name, "lo");
	ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
	if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0)
		sb_perr("ioctl(SIOCSIFFLAGS, lo) failed");
#endif
}

/* Create a nice empty /dev for playing in. */
static void ns_mount_setup(void)
{
#ifdef CLONE_NEWNS
	/* Create a new mount namespace. */
	if (unshare(CLONE_NEWNS))
		return;

	/* Mark the whole tree as private so we don't mess up the parent ns. */
	if (mount("none", "/", NULL, MS_PRIVATE | MS_REC, NULL))
		return;

	/* Create a unique /tmp dir for everyone. */
	if (mount("/tmp", "/tmp", "tmpfs", MS_NOSUID | MS_NODEV | MS_RELATIME, NULL))
		sb_ewarn("could not mount /tmp");

	/* Mount an empty dir inside of /dev which we'll populate with bind mounts
	 * to the existing files in /dev.  We can't just mknod ourselves because
	 * the kernel will deny those calls when we aren't actually root.  We pick
	 * the /dev/shm dir as it should generally exist and we don't care about
	 * binding its contents. */
	if (mount("sandbox-dev", "/dev/shm", "tmpfs", MS_NOSUID | MS_NOEXEC | MS_RELATIME, "mode=0755"))
		return;

	/* Now map in all the files/dirs we do want to expose. */
	int fd;
#define bind_file(node) \
	fd = open("/dev/shm/" node, O_CREAT, 0); \
	sb_assert(fd != -1); \
	close(fd); \
	xmount("/dev/" node, "/dev/shm/" node, NULL, MS_BIND, NULL)
#define bind_dir(node) \
	xmkdir("/dev/shm/" node, 0); \
	xmount("/dev/" node, "/dev/shm/" node, NULL, MS_BIND, NULL)

	bind_file("full");
	bind_file("null");
	bind_file("ptmx");
	bind_file("tty");
	bind_file("urandom");
	bind_file("zero");
	bind_dir("pts");

	xmkdir("/dev/shm/shm", 01777);
	xchmod("/dev/shm/shm", 01777);

	xsymlink("/proc/self/fd", "/dev/shm/fd");
	xsymlink("fd/0", "/dev/shm/stdin");
	xsymlink("fd/1", "/dev/shm/stdout");
	xsymlink("fd/2", "/dev/shm/stderr");

	xchmod("/dev/shm", 0555);

	/* Now that the new root looks good, move it to /dev. */
	xmount("/dev/shm", "/dev", NULL, MS_MOVE, NULL);
#endif
}

static pid_t ns_pid_setup(void)
{
	pid_t pid;

	if (unshare(CLONE_NEWPID) == 0) {
		/* Create a child in the new pid ns. */
		pid = fork();
		if (pid == 0) {
			/* Create a new mount namespace for the child. */
			sb_assert(unshare(CLONE_NEWNS) == 0);
			xmount("none", "/proc", NULL, MS_PRIVATE | MS_REC, NULL);
			xmount("proc", "/proc", "proc", MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_RELATIME, NULL);
		}
	} else {
		/* At least hide other procs. */
		if (umount2("/proc", MNT_FORCE | MNT_DETACH) == 0)
			xmount("proc", "/proc", "proc", MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_RELATIME, "hidepid=2");
		pid = fork();
	}

	return pid;
}

pid_t setup_namespaces(void)
{
	/* We need to unshare namespaces independently anyways as users can
	 * configure kernels to have only some enabled, and if we try to do
	 * them all at once, we'll get EINVAL. */

	int uid = getuid();
	int gid = getgid();
	pid_t pid;

	/* This comes first so we can do the others as non-root. */
	if (opt_use_ns_user)
		ns_user_switch(uid, gid, 0, 0);

#ifdef CLONE_NEWIPC
	if (opt_use_ns_ipc)
		unshare(CLONE_NEWIPC);
#endif
#ifdef CLONE_SYSVSEM
	if (opt_use_ns_sysv)
		unshare(CLONE_SYSVSEM);
#endif

#ifdef CLONE_NEWUTS
	if (opt_use_ns_uts && unshare(CLONE_NEWUTS) == 0) {
		const char name[] = "gentoo-sandbox";
		if (sethostname(name, sizeof(name) - 1))
			/* silence gcc warning */;
	}
#endif

	if (opt_use_ns_net)
		ns_net_setup();

	if (opt_use_ns_mnt)
		ns_mount_setup();

	if (opt_use_ns_mnt && opt_use_ns_pid)
		pid = ns_pid_setup();
	else
		pid = fork();

	if (opt_use_ns_user)
		ns_user_switch(0, 0, uid, gid);

	return pid;
}

#endif
