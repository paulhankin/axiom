/* flags: --c-std=gnu23 */
/* expect: 1 1 1 1 1 1 1 1 1 1 */
/* expect: 1 1 1 1 1 1 */
/* Bit-fields of __int128 and of _BitInt(N) for N > 64 (wider than one or
   two limbs, crossing limbs): stores and loads, sign extension, ++ and
   compound assignments, static and automatic initializers, the fields
   beside them kept. (Their layouts: c/run/wint_layout_*.c.) */
#include "host.h"

typedef __int128 i128;
typedef unsigned __int128 u128;

struct A {
    char c;
    i128 x : 100;
    i128 y : 60;
    u128 z : 3;
    long long tail;
};

struct B {
    long long a : 60;
    _BitInt(100) x : 70;
    unsigned _BitInt(200) y : 130;
    _BitInt(65) w : 2;
    char end;
};

static struct A sa = { 'k', -((i128)1 << 98), 12345, 5, 77 };
static struct B sb = { -3, ((_BitInt(100))1 << 68) + 9, ((unsigned _BitInt(200))1 << 129) | 1, -2, 'e' };

static char out[64];
static int nout;

static void ok(int c) {
    if (nout) out[nout++] = ' ';
    out[nout++] = c ? '1' : '0';
}

static void flush(void) {
    print_str(out, (size_t)nout);
    nout = 0;
}

int main(void) {
    struct A a = sa;
    ok(a.c == 'k' && a.x == -((i128)1 << 98) && a.y == 12345 && a.z == 5 && a.tail == 77);
    a.x = ((i128)1 << 99) - 1;
    ok(a.x == ((i128)1 << 99) - 1 && a.y == 12345 && a.c == 'k');
    a.x += 1;  /* (wraps to the least value of 100 bits) */
    ok(a.x == -((i128)1 << 99));
    a.y = -1;
    ok(a.y == -1 && a.z == 5 && a.tail == 77);
    a.z = 9;  /* (cut to 3 bits) */
    ok(a.z == 1);
    a.z++;
    ok(a.z == 2 && (a.y == -1));
    struct B b = sb;
    ok(b.a == -3 && b.x == ((_BitInt(100))1 << 68) + 9 && b.y == (((unsigned _BitInt(200))1 << 129) | 1));
    ok(b.w == -2 && b.end == 'e');
    b.y <<= 1;
    ok(b.y == 2 && b.x == ((_BitInt(100))1 << 68) + 9 && b.w == -2);
    b.x = -b.x;
    ok(b.x == -(((_BitInt(100))1 << 68) + 9) && b.a == -3 && b.end == 'e');
    flush();
    struct B c = { .y = ~(unsigned _BitInt(200))0, .x = -1 };
    ok(c.y == (((unsigned _BitInt(200))1 << 130) - 1) && c.x == -1 && c.a == 0 && c.w == 0);
    c.w = 1;
    ok(c.w == 1 && c.y == (((unsigned _BitInt(200))1 << 130) - 1));
    volatile struct A v = { 0 };
    v.x = 42;
    v.y = v.x * 2;
    ok(v.x == 42 && v.y == 84);
    ok(sizeof(struct A) % 16 == 0 && sizeof(struct A) >= 32 && sizeof(struct B) % _Alignof(struct B) == 0);
    i128 t = sa.y + sb.x;
    ok(t == 12345 + (((i128)1 << 68) + 9));
    sa.x = 3;
    ok(sa.x == 3 && sa.y == 12345);
    flush();
    return 0;
}
