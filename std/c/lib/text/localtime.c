/* localtime (C17 7.27.3.4): localtime_r into the object that gmtime and
   localtime share (7.27.3p1). */
#include <axc/text.h>
#include <time.h>

/* (a C23 function: this source is C17) */
struct tm *localtime_r(const time_t *timer, struct tm *buf);

struct tm *localtime(const time_t *timer) {
    return localtime_r(timer, __axc_tm_static());
}
