/* clock (C17 7.27.2.1): the processor time of the process (the runtime's
   clock 2), in microseconds (CLOCKS_PER_SEC); (clock_t)-1 where the
   system gives none (a negative count). */
#include <axc/text.h>
#include <time.h>

clock_t clock(void) {
    long long ns = __axc_clock_ns(2);
    if (ns < 0)
        return (clock_t)-1;
    return (clock_t)(ns / 1000);
}
