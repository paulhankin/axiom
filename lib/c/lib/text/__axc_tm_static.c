/* __axc_tm_static - the broken-down time that gmtime and localtime return
   (C17 7.27.3p1: one object, which each call may overwrite). */
#include <time.h>

struct tm *__axc_tm_static(void) {
    static struct tm tm;
    return &tm;
}
