/* flags: --c-std=gnu23 */
/* expect: 1 1 1 1 1 1 1 1 */
/* expect: 1 1 1 1 1 1 */
/* The integers wider than 64 bits in conditions (if, while, ?:, && and
   ||, !), as the operands of ?:, in compound literals, converted to and
   from pointers, in volatile objects, as array indexes and pointer offsets,
   typedefs of the attribute mode(TI), unary +, sizeof and _Alignof. */
#include <stdint.h>
#include "host.h"

typedef int ti_t __attribute__((mode(TI)));
typedef unsigned uti_t __attribute__((mode(TI)));

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

static volatile __int128 vz = 0;
static volatile unsigned _BitInt(300) vb = (unsigned _BitInt(300))1 << 299;

int main(void) {
    __int128 a = (__int128)1 << 64;
    _BitInt(200) b = 0;
    int n = 0;
    if (a) n++;
    if (!b) n++;
    while (a) {
        a >>= 16;
        n++;
    }
    ok(n == 2 + 5);
    ok((a || vb) && !(b && vz));
    __int128 x = vz ? (__int128)-1 : (__int128)1 << 100;
    ok(x == (__int128)1 << 100);
    unsigned _BitInt(300) y = vb ? vb : 0;
    ok(y == vb && y != 0);
    ok((unsigned _BitInt(300)){ 7 } * 3 == 21);
    int arr[4] = { 10, 20, 30, 40 };
    __int128 i = 2;
    ok(arr[i] == 30 && *(arr + (unsigned _BitInt(100))3) == 40 && (&arr[3] - (__int128)1)[0] == 30);
    uintptr_t p = (uintptr_t)&arr[1];
    unsigned __int128 q = (unsigned __int128)p;
    ok((int *)q == &arr[1] && (int *)(_BitInt(200))(intptr_t)&arr[2] == &arr[2]);
    ok(+x == x && -(-x) == x);
    flush();
    ok(sizeof(ti_t) == 16 && (ti_t)-1 < 0 && (uti_t)-1 > 0);
    ok(_Generic((ti_t)0, __int128: 1, default: 0) && _Generic((uti_t)0, unsigned __int128: 1, default: 0));
    ok(sizeof(__int128) == 16 && _Alignof(unsigned __int128) == 16 && sizeof(vb) >= 38);
    vz = 5;
    vz += 3;
    ok(vz == 8);
    __int128 k = 0;
    for (__int128 j = (__int128)1 << 70; j > 0; j >>= 10) k++;
    ok(k == 8);
    unsigned _BitInt(300) s = 0;
    for (int j = 0; j < 300; j += 3) s |= (unsigned _BitInt(300))1 << j;
    ok(__builtin_stdc_count_ones(s) == 100 && (s & 1) && !(s & 2));
    flush();
    return 0;
}
