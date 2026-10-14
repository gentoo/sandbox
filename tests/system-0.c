#define FUNC system
#define SFUNC "system"
#define FUNC_STR "\"%s\""
#define FUNC_IMP command
#define ARG_CNT 1
#define ARG_USE "<command>"
#define process_args() const char *command = argv[i++];
#include "test-skel-0.c"
