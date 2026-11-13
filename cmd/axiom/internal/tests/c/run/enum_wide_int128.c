/* flags: --c-std=gnu23 */
/* Enumeration constants wider than long long (C23 6.7.2.2, a GNU dialect,
   where __int128 is an extended integer type): with a fixed underlying type
   __int128 or unsigned __int128 any value of it; without one, the
   enumerated type is the first of int, long, long long, __int128 (unsigned
   int, unsigned long, unsigned long long, unsigned __int128 when no value
   is negative) that holds every value. In the list, a constant outside int
   has the type of its expression, or (an implicit one past its
   predecessor's type) a wider type of the same signedness; after the list
   every constant has the enumerated type, unless all fit int. */
#include "host.h"

typedef __int128 i128;
typedef unsigned __int128 u128;

static void print_u128(u128 x) {
    print_u64((unsigned long long)(x >> 64));
    print_u64((unsigned long long)x);
}

enum Fixed : i128 { F_BIG = (i128)1 << 100, F_NEXT, F_NEG = -((i128)1 << 90), F_MIN = (i128)((u128)1 << 127) };
/* C23: the same list again is the same type (6.7.2.3) */
enum Fixed : i128 { F_BIG = (i128)1 << 100, F_NEXT, F_NEG = -((i128)1 << 90), F_MIN = (i128)((u128)1 << 127) };
enum UFixed : u128 { U_TOP = ~(u128)0, U_LOW = 3 };

/* without a fixed type: past unsigned long long */
enum Past { P_LAST_ULL = 0xFFFFFFFFFFFFFFFFull, P_FIRST_WIDE, P_SIZE_IN = sizeof(P_FIRST_WIDE) };
/* a value from an expression of type __int128, and a negative one */
enum Mixed { M_NEG = -1, M_WIDE = (i128)1 << 64, M_TYPE_IN = _Generic(M_WIDE, i128: 1, default: 0) };
/* in the list, an implicit constant past long's range takes long long or __int128 */
enum Chain { C_LL = 0x7FFFFFFFFFFFFFFFll, C_NEXT, C_NEXT_IS = _Generic(C_NEXT, i128: 1, default: 0) };
/* all in int: the constants are int */
enum Small { S0, S1 = (i128)5 };
/* in the list, a wide _BitInt value keeps its type; the enumerated type is not bit-precise */
enum Precise { Q1 = (_BitInt(100))1 << 80, Q1_IS = _Generic(Q1, _BitInt(100): 1, default: 0) };
/* values from a _BitInt of more than 128 bits that fit unsigned __int128 */
enum FromBitInt { B_V = (unsigned _BitInt(200))1 << 127 };

int main(void) {
    print_u128(F_BIG);
    print_u128(F_NEXT);
    print_i64((long long)(F_NEG >> 64));
    print_u128(F_MIN);
    print_i64(sizeof(enum Fixed));
    print_i64(_Generic(F_BIG, enum Fixed: 1, default: 0));
    print_u128(U_TOP);
    print_i64(U_LOW > U_TOP);
    print_u128(P_FIRST_WIDE);
    print_i64(P_SIZE_IN);
    print_i64(_Generic((enum Past)0, u128: 1, default: 0));
    print_i64(_Generic(P_LAST_ULL, enum Past: 1, default: 0));
    print_i64(M_TYPE_IN);
    print_i64(_Generic((enum Mixed)0, i128: 1, default: 0));
    print_i64(M_NEG < M_WIDE);
    print_u128(M_WIDE);
    print_i64(C_NEXT_IS);
    print_u128(C_NEXT);
    print_i64(_Generic(S1, int: 1, default: 0));
    print_u128(B_V);
    print_i64(_Generic((enum FromBitInt)0, u128: 1, default: 0));
    print_i64(Q1_IS);
    print_i64(_Generic(Q1, enum Precise: 1, default: 0) + _Generic((enum Precise)0, u128: 1, default: 0));
    /* in a switch, and as a static initializer */
    static const i128 t[] = { F_BIG, F_NEG };
    volatile i128 v = F_NEXT;
    switch (v) {
    case F_BIG:
        print_i64(1);
        break;
    case F_NEXT:
        print_i64(2);
        break;
    default:
        print_i64(3);
    }
    print_i64(t[0] == F_BIG && t[1] == F_NEG);
    return 0;
}
/* expect: 68719476736 */
/* expect: 0 */
/* expect: 68719476736 */
/* expect: 1 */
/* expect: -67108864 */
/* expect: 9223372036854775808 */
/* expect: 0 */
/* expect: 16 */
/* expect: 1 */
/* expect: 18446744073709551615 */
/* expect: 18446744073709551615 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 16 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 9223372036854775808 */
/* expect: 1 */
/* expect: 9223372036854775808 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 2 */
/* expect: 1 */
