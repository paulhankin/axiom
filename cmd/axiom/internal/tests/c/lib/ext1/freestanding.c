/* --c-freestanding: Annex K is the hosted library's, so __STDC_LIB_EXT1__
   is not defined (6.10.8.3); <stddef.h> and <stdint.h> still give rsize_t
   and RSIZE_MAX when asked (they need no library). */
/* flags: --c-freestanding */
#define __STDC_WANT_LIB_EXT1__ 1
#include <stddef.h>
#include <stdint.h>
#ifdef __STDC_LIB_EXT1__
#error "__STDC_LIB_EXT1__ is defined"
#endif
static rsize_t r = RSIZE_MAX;
void print_u64(unsigned __axiom_int64 x);
int main(void) {
    print_u64(r == SIZE_MAX / 2);
    return 0;
}
/* expect: 1 */
