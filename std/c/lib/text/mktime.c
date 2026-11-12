/* mktime (C17 7.27.2.3): the fields of *timeptr may be out of their
   ranges (they are normalized); tm_isdst says whether it is a time of
   daylight saving time (> 0), of standard time (0), or not known (< 0),
   as __axc_tz_from_local reads it. On success every field is set,
   normalized, tm_isdst as the zone has it; (time_t)-1 when the result's
   year does not fit tm_year (*timeptr unchanged). */
#include <axc/text.h>
#include <time.h>

/* (a C23 function: this source is C17) */
struct tm *localtime_r(const time_t *timer, struct tm *buf);

time_t mktime(struct tm *timeptr) {
    time_t t = (time_t)__axc_tz_from_local(__axc_tm_to_seconds(timeptr), timeptr->tm_isdst);
    struct tm r;
    if (!localtime_r(&t, &r))
        return (time_t)-1;
    *timeptr = r;
    return t;
}
