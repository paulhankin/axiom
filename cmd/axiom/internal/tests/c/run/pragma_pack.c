/* flags: --c-std=c11 */
/* #pragma pack in a mode that is no GNU dialect (pragmas are in every
   mode): members aligned to at most n bytes, bit-fields placed at the next
   bit (Windows: in units of their type aligned to at most n, whose layout
   numbers the test expects there instead); push and pop. */
#include "host.h"
#include <stddef.h>

/* the layout expected: System V's and AAPCS64's, or Windows x64's
   (confirmed against the platforms' compilers) */
#define LAYOUT(v, sysv, win) print_i64((v) == (__AXIOM_BITFIELD_LAYOUT__ == 2 ? (win) : (sysv)) ? 1 : -(long long)(v))

#pragma pack(push, 2)
struct A { char c; int b : 20; int d : 12; char e; };
struct A2 { char c; int x; short y; };
#pragma pack(push, 1)
struct D { char c; int b : 20; long long q; };
#pragma pack(pop)
struct A3 { char c; long long q; };
#pragma pack(pop)
struct N { char c; int x; };

int main(void) {
    LAYOUT(sizeof(struct A) * 10 + _Alignof(struct A), 62, 82);
    print_i64(sizeof(struct A2) * 100 + _Alignof(struct A2) * 10 + offsetof(struct A2, y));
    LAYOUT(sizeof(struct D) * 100 + _Alignof(struct D) * 10 + offsetof(struct D, q), 1214, 1315);
    print_i64(sizeof(struct A3) * 10 + _Alignof(struct A3));
    print_i64(sizeof(struct N));
    struct A a = { 0 };
    a.b = 0x12345;
    a.d = 0x7ff;
    a.e = 9;
    unsigned char *p = (unsigned char *)&a;
    long long bytes = 0;
    for (unsigned i = 0; i < sizeof a; i++)
        bytes = bytes * 256 + p[i];
    LAYOUT(bytes, 296955772681LL, 76020677806336LL);
    print_i64(a.b + a.d + a.e);
    struct D d = { 1, -5, 0x123456789abLL };
    d.q += 1;
    print_i64(d.b * 1000 + (d.q == 0x123456789acLL));
    struct A2 arr[3] = { { 1, 100, 7 }, { 2, 200, 8 }, { 3, 300, 9 } };
    int s = 0;
    for (int i = 0; i < 3; i++)
        s += arr[i].x + arr[i].y;
    print_i64(s);
    return 0;
}
/* expect: 1 */
/* expect: 826 */
/* expect: 1 */
/* expect: 102 */
/* expect: 8 */
/* expect: 1 */
/* expect: 76621 */
/* expect: -4999 */
/* expect: 624 */
