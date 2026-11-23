/* asctime (C17 7.27.3.1; deprecated in C23): __axc_asctime. */
#include <axc/text.h>
#include <time.h>

char *asctime(const struct tm *timeptr) {
    return __axc_asctime(timeptr);
}
