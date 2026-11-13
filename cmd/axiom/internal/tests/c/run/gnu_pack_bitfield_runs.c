/* flags: --c-std=gnu23 */
/* not-windows */
/* Bit-fields of a packed structure that no unit of 1, 2, 4 or 8 bytes in it
   holds (System V, AAPCS64 and WebAssembly place them at the next bit), and
   those of types wider than 64 bits: each reached through the bytes it
   spans and no others (the members around it keep their values), loaded,
   stored, in static initializers, volatile. (Windows keeps them in units of
   their types: gnu_pack_bitfield_windows.c.) */
#include "host.h"

struct __attribute__((packed)) A { int b : 20; char c : 4; };
struct __attribute__((packed)) B { char c; unsigned long long x : 4; long long y : 64; char d; };
struct __attribute__((packed)) C { char c; unsigned __int128 x : 3; __int128 y : 120; char d; };
struct __attribute__((packed)) D { unsigned char f : 5; unsigned _BitInt(200) z : 190; char d; };
struct __attribute__((packed)) E { char c; unsigned __int128 t : 9; };

static struct B sb = { 7, 9, -12345678901234567, 66 };
static struct C sc = { 1, 5, -3, 2 };

static unsigned sum(const void *p, unsigned n) {
    const unsigned char *q = p;
    unsigned s = 0;
    for (unsigned i = 0; i < n; i++)
        s = s * 31 + q[i];
    return s;
}

int main(void) {
    struct A a = { -300000, 5 };
    print_i64(sizeof a);
    print_i64(a.b);
    a.b += 1000;
    print_i64(a.b * 100 + a.c);

    print_i64(sizeof(struct B));
    print_i64(sb.y);
    print_i64(sb.c + sb.x + sb.d);
    struct B b = sb;
    b.y = 0x123456789abcdef0;
    b.x = 3;
    print_u64((unsigned long long)b.y);
    print_i64(b.c * 1000 + b.x * 100 + b.d);
    print_u64(sum(&b, sizeof b));

    print_i64(sizeof(struct C));
    print_i64((long long)sc.y + sc.x + sc.c + sc.d);
    struct C c = sc;
    c.y = ((__int128)0x7fedcba987654321 << 48) | 0xabcdef;
    c.x = 6;
    print_u64((unsigned long long)(c.y >> 64));
    print_u64((unsigned long long)c.y);
    print_i64(c.c * 1000 + c.x * 100 + c.d);
    print_u64(sum(&c, sizeof c));

    struct D d = { 17, 0, 99 };
    d.z = ((unsigned _BitInt(200))1 << 189) | 0x5555;
    print_i64(sizeof d);
    print_i64(d.f + d.d);
    print_u64((unsigned long long)(d.z >> 186));
    print_u64((unsigned long long)d.z);
    d.z += 1;
    print_u64((unsigned long long)d.z);

    volatile struct E e = { 3, 0 };
    e.t = 300;
    e.t += 100;
    print_i64(sizeof e);
    print_i64((int)e.t + e.c);
    return 0;
}
/* expect: 3 */
/* expect: -300000 */
/* expect: -29899995 */
/* expect: 11 */
/* expect: -12345678901234567 */
/* expect: 82 */
/* expect: 1311768467463790320 */
/* expect: 7366 */
/* expect: 3484327662 */
/* expect: 18 */
/* expect: 5 */
/* expect: 140659300861797 */
/* expect: 4837147474783882735 */
/* expect: 1602 */
/* expect: 3461469768 */
/* expect: 26 */
/* expect: 116 */
/* expect: 8 */
/* expect: 21845 */
/* expect: 21846 */
/* expect: 3 */
/* expect: 403 */
