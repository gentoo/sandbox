#include "headers.h"

#define _msg(std, fmt, args...) fprintf(std, "%s:%s():%i: " fmt "\n", __FILE__, __func__, __LINE__, ##args)
#define _stderr_msg(fmt, args...) _msg(stderr, fmt, ##args)
#define _stderr_pmsg(fmt, args...) _msg(stderr, fmt ": %s", ##args, strerror(errno))
#define err(fmt, args...) ({ _stderr_msg(fmt, ##args); exit(1); })
#define errp(fmt, args...) ({ _stderr_pmsg(fmt, ##args); exit(1); })
