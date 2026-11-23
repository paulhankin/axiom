/* difftime (C17 7.27.2.2): time1 - time0, computed exactly in 64 bits
   (unsigned, as the difference of two time_t may not fit a time_t) and
   rounded once to double. */
#include <time.h>

double difftime(time_t time1, time_t time0) {
    if (time1 >= time0)
        return (double)((unsigned long long)time1 - (unsigned long long)time0);
    return -(double)((unsigned long long)time0 - (unsigned long long)time1);
}
