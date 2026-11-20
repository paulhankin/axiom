/* --c-lib=system: a system header's definition of a macro the program
   defined before including it (a fallback of the program's own, `assert`
   before <assert.h>) replaces the program's, without a diagnostic. */
/* flags: --target=x86_64 --c-lib=system --c-system-include=sysredef */
#define NARROW_MAX 100
#include <second.h>
int n = NARROW_MAX;
int w = WIDTH_MAX;
