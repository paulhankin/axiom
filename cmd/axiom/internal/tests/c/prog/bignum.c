/* flags: --c-std=c17 */
/* Arbitrary-precision unsigned and signed integers in 32-bit limbs:
   addition, subtraction, schoolbook and Karatsuba multiplication, long
   division by a limb and by a big number (Knuth's algorithm D), shifts,
   conversion to and from decimal and other bases, factorials, modular
   exponentiation, the greatest common divisor, and a primality test.
   Results are checked against identities (a = q*b + r, Karatsuba equals
   schoolbook, conversions round-trip) and their digits hashed. */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MAXL 400

typedef struct {
    int n;               /* limbs in use; no leading zero limbs */
    uint32_t d[MAXL];
} big;

static void norm(big *a) {
    while (a->n > 0 && a->d[a->n - 1] == 0) a->n--;
}

static void set_u64(big *a, uint64_t v) {
    a->n = 0;
    while (v) {
        a->d[a->n++] = (uint32_t)v;
        v >>= 32;
    }
}

static int cmp(const big *a, const big *b) {
    if (a->n != b->n) return a->n < b->n ? -1 : 1;
    for (int i = a->n - 1; i >= 0; i--)
        if (a->d[i] != b->d[i]) return a->d[i] < b->d[i] ? -1 : 1;
    return 0;
}

static void add(big *r, const big *a, const big *b) {
    int n = a->n > b->n ? a->n : b->n;
    uint64_t carry = 0;
    for (int i = 0; i < n; i++) {
        uint64_t s = carry;
        if (i < a->n) s += a->d[i];
        if (i < b->n) s += b->d[i];
        r->d[i] = (uint32_t)s;
        carry = s >> 32;
    }
    r->n = n;
    if (carry) r->d[r->n++] = (uint32_t)carry;
}

/* r = a - b, a >= b */
static void sub(big *r, const big *a, const big *b) {
    int64_t borrow = 0;
    for (int i = 0; i < a->n; i++) {
        int64_t s = (int64_t)a->d[i] - borrow - (i < b->n ? (int64_t)b->d[i] : 0);
        borrow = s < 0;
        r->d[i] = (uint32_t)(s + (borrow << 32));
    }
    r->n = a->n;
    norm(r);
}

static void mul_school(big *r, const big *a, const big *b) {
    big t;
    int n = a->n + b->n;
    for (int i = 0; i < n; i++) t.d[i] = 0;
    for (int i = 0; i < a->n; i++) {
        uint64_t carry = 0;
        for (int j = 0; j < b->n; j++) {
            uint64_t p = (uint64_t)a->d[i] * b->d[j] + t.d[i + j] + carry;
            t.d[i + j] = (uint32_t)p;
            carry = p >> 32;
        }
        t.d[i + b->n] = (uint32_t)carry;
    }
    t.n = n;
    norm(&t);
    *r = t;
}

static void shift_limbs(big *r, const big *a, int k) {
    for (int i = a->n - 1; i >= 0; i--) r->d[i + k] = a->d[i];
    for (int i = 0; i < k; i++) r->d[i] = 0;
    r->n = a->n ? a->n + k : 0;
}

static void slice(big *r, const big *a, int from, int count) {
    r->n = 0;
    for (int i = 0; i < count && from + i < a->n; i++) r->d[r->n++] = a->d[from + i];
    norm(r);
}

static int kara_calls;

static void mul_kara(big *r, const big *a, const big *b) {
    kara_calls++;
    if (a->n < 8 || b->n < 8) { mul_school(r, a, b); return; }
    int m = (a->n > b->n ? a->n : b->n) / 2;
    static big pool[64];
    static int top;
    if (top + 7 > 64) { mul_school(r, a, b); return; }
    big *a0 = &pool[top++], *a1 = &pool[top++], *b0 = &pool[top++], *b1 = &pool[top++];
    big *z0 = &pool[top++], *z1 = &pool[top++], *z2 = &pool[top++];
    slice(a0, a, 0, m);
    slice(a1, a, m, a->n);
    slice(b0, b, 0, m);
    slice(b1, b, m, b->n);
    mul_kara(z0, a0, b0);
    mul_kara(z2, a1, b1);
    add(a0, a0, a1);
    add(b0, b0, b1);
    mul_kara(z1, a0, b0);
    sub(z1, z1, z0);
    sub(z1, z1, z2);
    shift_limbs(a1, z2, 2 * m);
    shift_limbs(b1, z1, m);
    add(r, a1, b1);
    add(r, r, z0);
    top -= 7;
}

static uint32_t div_small(big *q, const big *a, uint32_t v) {
    uint64_t rem = 0;
    for (int i = a->n - 1; i >= 0; i--) {
        uint64_t cur = (rem << 32) | a->d[i];
        q->d[i] = (uint32_t)(cur / v);
        rem = cur % v;
    }
    q->n = a->n;
    norm(q);
    return (uint32_t)rem;
}

static void mul_small(big *r, const big *a, uint32_t v, uint32_t addend) {
    uint64_t carry = addend;
    for (int i = 0; i < a->n; i++) {
        uint64_t p = (uint64_t)a->d[i] * v + carry;
        r->d[i] = (uint32_t)p;
        carry = p >> 32;
    }
    r->n = a->n;
    if (carry) r->d[r->n++] = (uint32_t)carry;
}

static int clz32(uint32_t x) {
    int n = 0;
    if (!x) return 32;
    while (!(x & 0x80000000u)) { x <<= 1; n++; }
    return n;
}

static void shl_bits(big *r, const big *a, int s) {
    if (s == 0) { *r = *a; return; }
    uint32_t carry = 0;
    for (int i = 0; i < a->n; i++) {
        uint32_t v = a->d[i];
        r->d[i] = (v << s) | carry;
        carry = v >> (32 - s);
    }
    r->n = a->n;
    if (carry) r->d[r->n++] = carry;
}

static void shr_bits(big *r, const big *a, int s) {
    if (s == 0) { *r = *a; return; }
    for (int i = 0; i < a->n; i++) {
        uint32_t hi = i + 1 < a->n ? a->d[i + 1] : 0;
        r->d[i] = (a->d[i] >> s) | (hi << (32 - s));
    }
    r->n = a->n;
    norm(r);
}

/* Knuth D: q = a / b, rem = a % b, b nonzero */
static void divmod(big *q, big *rem, const big *a, const big *b) {
    if (cmp(a, b) < 0) { q->n = 0; *rem = *a; return; }
    if (b->n == 1) {
        uint32_t r = div_small(q, a, b->d[0]);
        set_u64(rem, r);
        return;
    }
    static big u, v;
    int s = clz32(b->d[b->n - 1]);
    shl_bits(&v, b, s);
    shl_bits(&u, a, s);
    if (u.n == a->n) u.d[u.n++] = 0;
    else { u.d[u.n] = 0; }
    int n = v.n, m = u.n - n;
    for (int j = m - 1; j >= 0; j--) {
        uint64_t num = ((uint64_t)u.d[j + n] << 32) | u.d[j + n - 1];
        uint64_t qhat = num / v.d[n - 1];
        uint64_t rhat = num % v.d[n - 1];
        while (qhat >= ((uint64_t)1 << 32) || qhat * v.d[n - 2] > ((rhat << 32) | u.d[j + n - 2])) {
            qhat--;
            rhat += v.d[n - 1];
            if (rhat >= ((uint64_t)1 << 32)) break;
        }
        int64_t borrow = 0;
        uint64_t carry = 0;
        for (int i = 0; i < n; i++) {
            uint64_t p = qhat * v.d[i] + carry;
            carry = p >> 32;
            int64_t t = (int64_t)u.d[i + j] - borrow - (int64_t)(uint32_t)p;
            borrow = t < 0;
            u.d[i + j] = (uint32_t)(t + (borrow << 32));
        }
        int64_t t = (int64_t)u.d[j + n] - borrow - (int64_t)carry;
        borrow = t < 0;
        u.d[j + n] = (uint32_t)t;
        if (borrow) {
            qhat--;
            uint64_t c = 0;
            for (int i = 0; i < n; i++) {
                uint64_t s2 = (uint64_t)u.d[i + j] + v.d[i] + c;
                u.d[i + j] = (uint32_t)s2;
                c = s2 >> 32;
            }
            u.d[j + n] += (uint32_t)c;
        }
        q->d[j] = (uint32_t)qhat;
    }
    q->n = m;
    norm(q);
    u.n = n;
    norm(&u);
    shr_bits(rem, &u, s);
}

static int to_base(const big *a, unsigned base, char *out, int cap) {
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    big t = *a;
    int n = 0;
    if (t.n == 0) { out[n++] = '0'; out[n] = 0; return n; }
    while (t.n && n < cap - 1) out[n++] = digits[div_small(&t, &t, base)];
    for (int i = 0; i < n / 2; i++) {
        char c = out[i];
        out[i] = out[n - 1 - i];
        out[n - 1 - i] = c;
    }
    out[n] = 0;
    return n;
}

static void from_base(big *a, const char *s, unsigned base) {
    a->n = 0;
    for (; *s; s++) {
        unsigned c = (unsigned char)*s, v;
        if (c >= '0' && c <= '9') v = c - '0';
        else if (c >= 'a' && c <= 'z') v = c - 'a' + 10;
        else continue;
        mul_small(a, a, base, v);
        if (a->n == 0 && v) set_u64(a, v);
    }
}

static uint32_t hash_str(uint32_t h, const char *s, int n) {
    for (int i = 0; i < n; i++) h = (h ^ (unsigned char)s[i]) * 16777619u;
    return h;
}

static void powmod(big *r, const big *b, const big *e, const big *m) {
    static big base, acc, q, t;
    set_u64(&acc, 1);
    divmod(&q, &base, b, m);
    for (int i = e->n - 1; i >= 0; i--) {
        for (int bit = 31; bit >= 0; bit--) {
            mul_school(&t, &acc, &acc);
            divmod(&q, &acc, &t, m);
            if ((e->d[i] >> bit) & 1) {
                mul_school(&t, &acc, &base);
                divmod(&q, &acc, &t, m);
            }
        }
    }
    *r = acc;
}

static void gcd(big *r, const big *a, const big *b) {
    static big x, y, q, t;
    x = *a;
    y = *b;
    while (y.n) {
        divmod(&q, &t, &x, &y);
        x = y;
        y = t;
    }
    *r = x;
}

/* Miller-Rabin with fixed bases */
static bool probably_prime(const big *n) {
    static big one, nm1, d, x, q, t, a;
    set_u64(&one, 1);
    if (n->n == 0) return false;
    if (n->n == 1 && n->d[0] < 4) return n->d[0] >= 2;
    if (!(n->d[0] & 1)) return false;
    sub(&nm1, n, &one);
    d = nm1;
    int s = 0;
    while (!(d.d[0] & 1)) { shr_bits(&d, &d, 1); s++; }
    static const uint32_t bases[] = { 2, 3, 5, 7, 11, 13, 17 };
    for (int k = 0; k < 7; k++) {
        set_u64(&a, bases[k]);
        if (cmp(&a, &nm1) >= 0) continue;
        powmod(&x, &a, &d, n);
        if ((x.n == 1 && x.d[0] == 1) || cmp(&x, &nm1) == 0) continue;
        bool comp = true;
        for (int i = 1; i < s; i++) {
            mul_school(&t, &x, &x);
            divmod(&q, &x, &t, n);
            if (cmp(&x, &nm1) == 0) { comp = false; break; }
        }
        if (comp) return false;
    }
    return true;
}

static big A, B, C, D, Q, R;
static char text[4000];

int main(void) {
    uint32_t h = 2166136261u;

    /* 100! in decimal */
    set_u64(&A, 1);
    for (uint32_t i = 2; i <= 100; i++) mul_small(&A, &A, i, 0);
    int n = to_base(&A, 10, text, sizeof text);
    print_i64(n);
    print_str(text, 20);
    print_str(text + n - 30, 30);
    h = hash_str(h, text, n);

    /* 300! two ways must agree: by small multiplications and by Karatsuba products of halves */
    set_u64(&A, 1);
    for (uint32_t i = 2; i <= 150; i++) mul_small(&A, &A, i, 0);
    set_u64(&B, 1);
    for (uint32_t i = 151; i <= 300; i++) mul_small(&B, &B, i, 0);
    mul_kara(&C, &A, &B);
    mul_school(&D, &A, &B);
    print_i64(cmp(&C, &D));
    print_i64(C.n);
    n = to_base(&C, 10, text, sizeof text);
    print_i64(n);
    h = hash_str(h, text, n);
    print_i64(kara_calls);

    /* division identity: C = Q * A + R with R < A */
    divmod(&Q, &R, &C, &A);
    print_i64(cmp(&Q, &B));
    print_i64(R.n);
    set_u64(&D, 0xfedcba9876543211ull);
    add(&C, &C, &D);
    divmod(&Q, &R, &C, &A);
    mul_school(&D, &Q, &A);
    add(&D, &D, &R);
    print_i64(cmp(&D, &C));
    print_i64(cmp(&R, &A) < 0);
    n = to_base(&R, 16, text, sizeof text);
    h = hash_str(h, text, n);

    /* base conversions round-trip */
    for (unsigned base = 2; base <= 36; base += 5) {
        n = to_base(&C, base, text, sizeof text);
        from_base(&D, text, base);
        h = hash_str(h, text, n);
        print_i64(base * 10000 + n * 10 + (cmp(&C, &D) == 0));
    }

    /* 2^127 - 1 and 2^89 - 1 are prime, 2^67 - 1 is not */
    static const int exps[] = { 31, 61, 67, 89, 107, 127 };
    for (int k = 0; k < 6; k++) {
        set_u64(&A, 1);
        for (int i = 0; i < exps[k]; i++) mul_small(&A, &A, 2, 0);
        set_u64(&B, 1);
        sub(&A, &A, &B);
        print_bool(probably_prime(&A));
    }

    /* modular exponentiation: 3^(10^30) mod (10^40 + 7) */
    from_base(&A, "3", 10);
    from_base(&B, "1000000000000000000000000000000", 10);
    from_base(&C, "10000000000000000000000000000000000000007", 10);
    powmod(&D, &A, &B, &C);
    n = to_base(&D, 10, text, sizeof text);
    print_str(text, (size_t)n);

    /* gcd of two factorial-ish products */
    set_u64(&A, 1);
    for (uint32_t i = 1; i <= 60; i += 2) mul_small(&A, &A, i, 0);
    set_u64(&B, 1);
    for (uint32_t i = 3; i <= 90; i += 3) mul_small(&B, &B, i, 0);
    gcd(&C, &A, &B);
    n = to_base(&C, 10, text, sizeof text);
    print_str(text, (size_t)n);
    h = hash_str(h, text, n);

    /* Fibonacci 1000 */
    set_u64(&A, 0);
    set_u64(&B, 1);
    for (int i = 0; i < 1000; i++) {
        add(&C, &A, &B);
        A = B;
        B = C;
    }
    n = to_base(&A, 10, text, sizeof text);
    print_i64(n);
    print_str(text, 16);
    h = hash_str(h, text, n);

    /* shifts are inverse */
    shl_bits(&B, &A, 13);
    shr_bits(&C, &B, 13);
    print_i64(cmp(&A, &C));
    print_u64(h);
    return 0;
}

/* expect: 158 */
/* expect: 93326215443944152681 */
/* expect: 916864000000000000000000000000 */
/* expect: 0 */
/* expect: 64 */
/* expect: 615 */
/* expect: 37 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 40421 */
/* expect: 77281 */
/* expect: 125701 */
/* expect: 175001 */
/* expect: 224581 */
/* expect: 274301 */
/* expect: 324091 */
/* expect: true */
/* expect: true */
/* expect: false */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: 6333738996691065182068301069178342008928 */
/* expect: 3952575621190533915703125 */
/* expect: 209 */
/* expect: 4346655768693745 */
/* expect: 0 */
/* expect: 1479121429 */
