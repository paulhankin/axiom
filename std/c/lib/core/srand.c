/* srand (<stdlib.h>, C23 7.24.2.2): starts rand's sequence anew from the
   seed: the same seed, the same sequence; srand(1) the first one. */
#include <stdlib.h>
#include "axc/core_impl.h"

void srand(unsigned int seed) {
    __axc_rand_state = seed;
}
