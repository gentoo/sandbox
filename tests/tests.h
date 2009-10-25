#include "headers.h"

#define _msg(std, fmt, args...) fprintf(std, "%s:%s():%i: " fmt "\n", __FILE__, __func__, __LINE__, ##args)
#define _stderr_msg(fmt, args...) _msg(stderr, fmt, ##args)
#define _stderr_pmsg(fmt, args...) _msg(stderr, fmt ": %s", ##args, strerror(errno))
#define err(fmt, args...) ({ _stderr_msg(fmt, ##args); exit(1); })
#define errp(fmt, args...) ({ _stderr_pmsg(fmt, ##args); exit(1); })

typedef struct {
	const char *name;
	int val;
} value_pair;
#define PAIR(x) { #x, x },

int lookup_val(const value_pair *tbl, const char *name)
{
	size_t i;
	for (i = 0; tbl[i].name; ++i)
		if (!strcmp(name, tbl[i].name))
			return tbl[i].val;
	err("unable to locate '%s'", name);
}

const char *lookup_str(const value_pair *tbl, int val)
{
	size_t i;
	for (i = 0; tbl[i].name; ++i)
		if (tbl[i].val == val)
			return tbl[i].name;
	err("unable to locate '%i'", val);
}

#define make_lookups(section) \
int lookup_##section(const char *str) { return atoi(str) ? : lookup_val(tbl_##section, str); } \
const char *rev_lookup_##section(int val) { return lookup_str(tbl_##section, val); }

const value_pair tbl_errno[] = {
	{ "Success", 0 },
	PAIR(EACCES)
	PAIR(EBADF)
	PAIR(EEXIST)
	PAIR(EFAULT)
	PAIR(EINVAL)
	PAIR(EISDIR)
	PAIR(ELOOP)
	PAIR(ENAMETOOLONG)
	PAIR(ENODEV)
	PAIR(ENOENT)
	PAIR(ENOTDIR)
	PAIR(EPERM)
	PAIR(ETXTBSY)
	{ }
};
make_lookups(errno)

const value_pair tbl_signal[] = {
	{ "SIGEXIT", 0 },
	PAIR(SIGABRT)
	PAIR(SIGALRM)
	PAIR(SIGCHLD)
	PAIR(SIGCONT)
	PAIR(SIGHUP)
	PAIR(SIGILL)
	PAIR(SIGINT)
	PAIR(SIGKILL)
	PAIR(SIGPIPE)
	PAIR(SIGQUIT)
	PAIR(SIGSEGV)
	PAIR(SIGSTOP)
	PAIR(SIGTRAP)
	PAIR(SIGTERM)
	PAIR(SIGUSR1)
	PAIR(SIGUSR2)
	{ }
};
make_lookups(signal)
