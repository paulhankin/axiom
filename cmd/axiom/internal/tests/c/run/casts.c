/* Casts between the scalar types: integers of each width and signedness,
   floating types, pointers and integers, _Bool, void. */
#include "host.h"

int main(void) {
    long long v = 0x123456789ABCDEF0LL;
    double d = -2.75;
    int x = 5;
    int *p = &x;
    unsigned long long up;
    print_i64((char)v);
    print_i64((unsigned char)v);
    print_i64((short)v);
    print_i64((unsigned short)v);
    print_i64((int)v);
    print_u64((unsigned)v);
    print_i64((long long)(unsigned)v);
    print_i64((long long)(int)v);
    print_i64((_Bool)v);
    print_i64((_Bool)0.0);
    print_i64((_Bool)p);
    print_i64((int)d);
    print_i64((unsigned char)(int)d);
    print_f64((float)d);
    print_f64((double)(char)-3);
    print_f64((double)(unsigned char)-3);
    print_i64((double)0xFFFFFFFFFFFFFFFFull == 18446744073709551616.0);
    print_f64((float)0xFFFFFFFFull);
    print_i64((long long)(double)(1LL << 53));
    print_u64((unsigned long long)1.8e19);
    up = (unsigned long long)(size_t)p;
    print_i64((int *)(size_t)up == p);
    print_i64(*(int *)(void *)(char *)p);
    print_i64((size_t)(char *)0);
    (void)x;
    (void)(x + 1);
    print_i64((signed char)(unsigned char)255);
    print_i64((unsigned short)(short)-1);
    print_i64((int)(float)16777217);
    print_i64((int)'a' + (char)1);
    return 0;
}

/* expect: -16 */
/* expect: 240 */
/* expect: -8464 */
/* expect: 57072 */
/* expect: -1698898192 */
/* expect: 2596069104 */
/* expect: 2596069104 */
/* expect: -1698898192 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: -2 */
/* expect: 254 */
/* expect: -2.75 */
/* expect: -3 */
/* expect: 253 */
/* expect: 1 */
/* expect: 4294967296 */
/* expect: 9007199254740992 */
/* expect: 18000000000000000000 */
/* expect: 1 */
/* expect: 5 */
/* expect: 0 */
/* expect: -1 */
/* expect: 65535 */
/* expect: 16777216 */
/* expect: 98 */
