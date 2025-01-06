#define CONFIG HAVE___UTIMENSAT64 || HAVE___UTIMENSAT_TIME64
#define _FILE_OFFSET_BITS 64
#define _TIME_BITS 64
#include "utimensat-0.c"
