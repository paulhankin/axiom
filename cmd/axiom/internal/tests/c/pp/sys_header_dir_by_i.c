/* --c-lib=system: -I of one of the system's directories is left out (as a
   cc does), so their headers stay the system's: a redefinition one of them
   makes of another's macro replaces it, as without the -I. */
/* flags: --target=x86_64 --c-lib=system --c-system-include=sysredef -Isysredef/ */
#include <first.h>
#include <second.h>
int w = WIDTH_MAX;
