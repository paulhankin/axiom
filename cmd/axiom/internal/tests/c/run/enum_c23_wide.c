/* flags: --c-std=c23 --c-long=64 */
/* C23 6.7.2.2: enumeration constants outside int's range, up to those of
   unsigned long long (an implicit value past its predecessor's type takes a
   wider type of the same signedness); the enumerated type is compatible with an integer
   type that holds them all, and the constants have the enumerated type
   once the list is complete (int if they all fit it). */
#include "host.h"

enum Big { BIG = 0xFFFFFFFFFFFFFFFFull };
enum Next { LAST_UINT = 0xFFFFFFFFu, FIRST_ULONG };
enum Signed { LAST_INT = 0x7FFFFFFF, FIRST_LONG };
enum Small { S0, S1 = 5 };

int main(void) {
    print_u64(BIG);
    print_u64(FIRST_ULONG);
    print_i64(FIRST_LONG);
    print_i64(_Generic((enum Signed)0, unsigned int: 1, default: 0));
    print_i64(sizeof(enum Big));
    print_i64(_Generic(BIG, enum Big: 1, default: 0));
    print_i64(_Generic(S1, int: 1, default: 0));
    print_i64(_Generic((enum Big)0, unsigned long: 1, default: 0));
    return 0;
}
/* expect: 18446744073709551615 */
/* expect: 4294967296 */
/* expect: 2147483648 */
/* expect: 1 */
/* expect: 8 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
