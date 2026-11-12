/* atoll (<stdlib.h>, C23 7.24.1.2): (long long)strtoll(nptr, (char **)NULL, 10); a value
   it cannot represent is undefined: here, the conversion's of the clamped
   value. */
#include <stdlib.h>

long long atoll(const char *nptr) {
    return (long long)strtoll(nptr, (char **)0, 10);
}
