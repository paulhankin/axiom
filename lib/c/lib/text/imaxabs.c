/* imaxabs (C17 7.8.2.1). */
#include <inttypes.h>

intmax_t imaxabs(intmax_t j) {
    return j < 0 ? -j : j;
}
