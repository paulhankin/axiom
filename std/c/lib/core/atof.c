/* atof (<stdlib.h>, C23 7.24.1.1): strtod(nptr, (char **)NULL), except for
   errno, which it may change all the same. */
#include <stdlib.h>

double atof(const char *nptr) {
    return strtod(nptr, (char **)0);
}
