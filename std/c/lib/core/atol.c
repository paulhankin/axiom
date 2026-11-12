/* atol (<stdlib.h>, C23 7.24.1.2): (long)strtol(nptr, (char **)NULL, 10); a value
   it cannot represent is undefined: here, the conversion's of the clamped
   value. */
#include <stdlib.h>

long atol(const char *nptr) {
    return (long)strtol(nptr, (char **)0, 10);
}
