/* ctime (C17 7.27.3.2; deprecated in C23): asctime(localtime(timer)); a null pointer when
   localtime gives none (C23 7.29.3.2p2). */
#include <axc/text.h>
#include <time.h>

char *ctime(const time_t *timer) {
    struct tm *tm = localtime(timer);
    return tm ? __axc_asctime(tm) : 0;
}
