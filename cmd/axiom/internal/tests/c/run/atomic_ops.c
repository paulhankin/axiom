/* flags: --c-std=c11 */
/* Atomic objects (C11 6.2.5p27, 6.5.2.4p2, 6.5.16.2p3): loads, stores,
   ++, -- and every compound assignment on each kind of atomic type, with
   their values (the new one, or for postfix the old), in one thread. */
#include <complex.h>
#include <stdatomic.h>
#include "host.h"

struct pair { short a, b; };   /* 4 bytes: lock-free */
struct three { char c[3]; };   /* 3 bytes: under the lock */
struct big { long long x, y; };  /* 16 bytes: lock-free on x86-64 and AArch64 (aligned to 16) */

_Atomic int ai = 5;
_Atomic(unsigned char) uc = 250;
_Atomic signed char sc = 120;
_Atomic _Bool flag;
_Atomic long long ll = -3;
_Atomic float f = 1.5f;
_Atomic double d = 2.25;
int arr[10];
int *_Atomic ap = &arr[2];
_Atomic struct pair pr = { 1, 2 };
_Atomic struct three th = { { 'a', 'b', 'c' } };
_Atomic struct big bg = { 7, 8 };
/* long double in the target's format: wider than double (16 bytes, its
   arithmetic in software, lock-free on x86-64 and AArch64 Linux, under the
   lock on WebAssembly) on Linux and WebAssembly, double's (8 bytes,
   lock-free) on Windows and macOS */
_Atomic long double ld = 1.5L;
_Atomic long double _Complex lz = CMPLXL(2.0L, 1.0L);

int main(void) {
    print_i64(ai++);
    print_i64(ai);
    print_i64(++ai);
    print_i64(ai--);
    print_i64(--ai);
    print_i64(ai += 10);
    print_i64(ai -= 3);
    print_i64(ai *= 4);
    print_i64(ai /= 3);
    print_i64(ai %= 7);
    print_i64(ai <<= 4);
    print_i64(ai >>= 2);
    print_i64(ai |= 0x100);
    print_i64(ai &= 0x1f0);
    print_i64(ai ^= 0xff);
    /* narrow types wrap in their own width */
    print_i64(uc += 10);
    print_i64(sc += 10);
    print_i64(uc * 2);
    /* _Bool: any nonzero result is 1 */
    print_i64(flag++);
    print_i64(flag += 5);
    print_i64(flag -= 1);
    print_i64(flag *= 7);
    print_i64(ll -= 1000000000000LL);
    print_i64(ll *= -2);
    /* floating: a loop of compare-exchanges */
    print_f64(f += 0.25f);
    print_f64(f *= 4);
    print_f64(f++);
    print_f64(d /= 2);
    print_f64(--d);
    /* pointers move by elements */
    ap += 3;
    print_i64(ap - arr);
    print_i64(ap++ - arr);
    print_i64(--ap - arr);
    ap -= 5;
    print_i64(ap - arr);
    /* structures and unions: whole loads and stores */
    {
        struct pair q = pr;
        struct three t;
        struct big b;
        print_i64(q.a * 10 + q.b);
        q.a = 30;
        q.b = 40;
        pr = q;
        q = pr;
        print_i64(q.a + q.b);
        t = th;
        print_str(t.c, 3);
        t.c[1] = 'X';
        th = t;
        t.c[1] = 'Y';
        t = th;
        print_str(t.c, 3);
        b = bg;
        print_i64(b.x * 100 + b.y);
        b.x = -1;
        bg = b;
        b = bg;
        print_i64(b.x);
    }
    /* the value of an assignment is the value stored */
    print_i64((ai = 77) + 1);
    /* (3, or on the platforms that pad atomic types 4: atomic_layout_<target>.c) */
    print_i64((sizeof(_Atomic struct three) == 3 || sizeof(_Atomic struct three) == 4) * 30 + _Alignof(_Atomic struct pair));
    {
        long double _Complex w;
        long double e = 9.0L;
        print_f64((double)(ld += 0.25L));
        print_f64((double)(ld *= 3));
        print_f64((double)ld--);
        print_f64((double)++ld);
        print_f64((double)atomic_exchange(&ld, 0.5L));
        print_i64(atomic_compare_exchange_strong(&ld, &e, 4.0L));
        print_f64((double)e);
        lz += 1.0L;
        lz *= CMPLXL(0.0L, 2.0L);
        w = lz;
        print_f64((double)__builtin_creal(w));
        print_f64((double)__builtin_cimag(w));
        /* (lock-free: 8 bytes; 16 on x86-64 (cmpxchg16b) and AArch64 Linux (exclusive pairs), not WebAssembly) */
        print_i64(atomic_is_lock_free(&ld) == (__AXIOM_LDBL_WIDTH__ == 64 || __AXIOM_LDBL_WIDTH__ == 80 ||
                                               __AXIOM_BITFIELD_LAYOUT__ == 1));
        /* (lock-free where it is 16 bytes, long double being double's format: Windows, AArch64 macOS) */
        print_i64(atomic_is_lock_free(&lz) == (__AXIOM_LDBL_WIDTH__ == 64));
    }
    return 0;
}
/* expect: 5 */
/* expect: 6 */
/* expect: 7 */
/* expect: 7 */
/* expect: 5 */
/* expect: 15 */
/* expect: 12 */
/* expect: 48 */
/* expect: 16 */
/* expect: 2 */
/* expect: 32 */
/* expect: 8 */
/* expect: 264 */
/* expect: 256 */
/* expect: 511 */
/* expect: 4 */
/* expect: -126 */
/* expect: 8 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */
/* expect: -1000000000003 */
/* expect: 2000000000006 */
/* expect: 1.75 */
/* expect: 7 */
/* expect: 7 */
/* expect: 1.125 */
/* expect: 0.125 */
/* expect: 5 */
/* expect: 5 */
/* expect: 5 */
/* expect: 0 */
/* expect: 12 */
/* expect: 70 */
/* expect: abc */
/* expect: aXc */
/* expect: 708 */
/* expect: -1 */
/* expect: 78 */
/* expect: 34 */
/* expect: 1.75 */
/* expect: 5.25 */
/* expect: 5.25 */
/* expect: 5.25 */
/* expect: 5.25 */
/* expect: 0 */
/* expect: 0.5 */
/* expect: -2 */
/* expect: 6 */
/* expect: 1 */
/* expect: 1 */
