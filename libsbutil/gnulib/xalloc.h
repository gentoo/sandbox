#include <sbutil.h>
/* make canonicalize use unwrapped faccessat */
#define faccessat sbio_faccessat
