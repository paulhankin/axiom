/* rand (<stdlib.h>, C23 7.24.2.1): the next number of a 64-bit linear
   congruential generator, x' = a x + c mod 2^64 with Knuth's multiplier
   a = 6364136223846793005 and c = 1442695040888963407 (The Art of
   Computer Programming, vol. 2, 3.3.4), whose high bits are the best:
   bits 33 to 63, 0 to RAND_MAX. */
#include <stdlib.h>
#include "axc/core_impl.h"

int rand(void) {
    __axc_rand_state = __axc_rand_state * 6364136223846793005ULL + 1442695040888963407ULL;
    return (int)(__axc_rand_state >> 33);
}
