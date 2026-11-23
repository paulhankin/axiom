/* gmtime (C17 7.27.3.3): gmtime_r into the object that gmtime and
   localtime share (7.27.3p1). */
#include <axc/text.h>
#include <time.h>

/* (a C23 function: this source is C17) */
struct tm *gmtime_r(const time_t *timer, struct tm *buf);

struct tm *gmtime(const time_t *timer) {
    return gmtime_r(timer, __axc_tm_static());
}
