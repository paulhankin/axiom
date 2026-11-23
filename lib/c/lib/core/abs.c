/* abs (<stdlib.h>, C23 7.24.6.1): the absolute value of j (undefined when it
   cannot be represented: the most negative value). */
#include <stdlib.h>

int abs(int j) {
    return j < 0 ? -j : j;
}
