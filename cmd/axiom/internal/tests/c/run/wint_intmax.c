/* flags: --c-std=gnu23 */
/* check-only */
/* intmax_t stays 64 bits beside __int128 and _BitInt (C23 7.22.1.5: it need
   not hold an extended or bit-precise type's values), and so does the
   preprocessor's arithmetic (6.10.2). */
#include <stdint.h>
#include <inttypes.h>
static_assert(sizeof(intmax_t) == 8 && INTMAX_MAX == 0x7fffffffffffffff && UINTMAX_WIDTH == 64);
#if INTMAX_MAX != 9223372036854775807
#error
#endif
#if -1 > 0u
#else
#error "preprocessor arithmetic not uintmax_t"
#endif
#if 18446744073709551615u != -1
#error
#endif
int main(void) { return 0; }
