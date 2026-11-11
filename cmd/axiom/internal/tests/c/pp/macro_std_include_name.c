/* A header name made by macro replacement (C99 6.10.3.5 EXAMPLE 4's xstr(INCFILE(2).h)). */
/* flags: --c-std=c99 */
#define str(s) # s
#define xstr(s) str(s)
#define INCFILE(n) inc/vers ## n
#include xstr(INCFILE(2).h)
version
