#include "headers.h"

#define _msg(std, fmt, args...) fprintf(std, "%s:%s():%i: " fmt "\n", __FILE__, __func__, __LINE__, ##args)
#define _stderr_msg(fmt, args...) _msg(stderr, fmt, ##args)
#define err(fmt, args...) ({ _stderr_msg(fmt, ##args); exit(1); })
