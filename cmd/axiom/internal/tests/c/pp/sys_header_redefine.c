/* --c-lib=system: a system header's redefinition of a macro another system
   header defined replaces it (the later definition holds), without a
   diagnostic. */
/* flags: --target=x86_64 --c-lib=system --c-system-include=sysredef */
#include <first.h>
#include <second.h>
int w = WIDTH_MAX;
int s = SAME_EVERYWHERE;
