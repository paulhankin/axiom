/* atoi (<stdlib.h>, C23 7.24.1.2): (int)strtol(nptr, (char **)NULL, 10); a value
   it cannot represent is undefined: here, the conversion's of the clamped
   value. */
#include <stdlib.h>

int atoi(const char *nptr) {
    return (int)strtol(nptr, (char **)0, 10);
}
