/* flags: --c-std=c11 --c-long=32 */
/* A program written for a 32-bit long (whatever the pointers' width): a
   CRC-32 computed bit by bit and by table in unsigned long, a 32-bit
   multiplicative hash with its avalanche checked by bit counts, a
   xorshift generator and a linear congruential one in unsigned long,
   Q16.16 fixed point in long with long long products, calendar
   arithmetic (days from a civil date and back) in long, and a 32-bit
   integer-to-text conversion. The first section checks what a 32-bit long
   implies: limits, the types of constants and of mixed long and unsigned
   int arithmetic, and that long long is still 64 bits. */
#include "host.h"
#include <limits.h>
#include <stdint.h>

#define KIND(x) _Generic((x), int: 1, unsigned: 2, long: 3, unsigned long: 4, long long: 5, unsigned long long: 6, \
                         default: 0)

static unsigned long crc_bitwise(const unsigned char *p, int n) {
    unsigned long c = 0xFFFFFFFFUL;
    for (int i = 0; i < n; i++) {
        c ^= p[i];
        for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xEDB88320UL & (0UL - (c & 1UL)));
    }
    return c ^ 0xFFFFFFFFUL;
}

static unsigned long crc_table[256];
static unsigned long crc_tabled(const unsigned char *p, int n) {
    unsigned long c = 0xFFFFFFFFUL;
    for (int i = 0; i < n; i++) c = crc_table[(c ^ p[i]) & 0xFFUL] ^ (c >> 8);
    return c ^ 0xFFFFFFFFUL;
}

static unsigned long mix32(unsigned long h) {
    h ^= h >> 16;
    h *= 0x85EBCA6BUL;
    h ^= h >> 13;
    h *= 0xC2B2AE35UL;
    h ^= h >> 16;
    return h;
}

static int bits(unsigned long x) {
    int n = 0;
    while (x) { n += (int)(x & 1UL); x >>= 1; }
    return n;
}

static unsigned long xs = 2463534242UL;
static unsigned long xorshift32(void) {
    xs ^= xs << 13;
    xs ^= xs >> 17;
    xs ^= xs << 5;
    return xs;
}

typedef long fix;   /* Q16.16 */
#define FIX(x) ((fix)((x) * 65536.0))
static fix fmul(fix a, fix b) { return (fix)(((long long)a * b) >> 16); }
static fix fdiv(fix a, fix b) { return (fix)(((long long)a * 65536) / b); }
static fix fsqrt(fix x) {
    /* Newton in fixed point */
    fix g = x > FIX(1) ? x / 2 : FIX(1);
    for (int i = 0; i < 30; i++) {
        fix n = (g + fdiv(x, g)) / 2;
        if (n == g) break;
        g = n;
    }
    return g;
}

/* days since 1970-01-01 of a proleptic Gregorian date */
static long days_from_civil(long y, long m, long d) {
    y -= m <= 2;
    long era = (y >= 0 ? y : y - 399) / 400;
    long yoe = y - era * 400;
    long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

static void civil_from_days(long z, long *y, long *m, long *d) {
    z += 719468;
    long era = (z >= 0 ? z : z - 146096) / 146097;
    long doe = z - era * 146097;
    long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    long mp = (5 * doy + 2) / 153;
    *d = doy - (153 * mp + 2) / 5 + 1;
    *m = mp + (mp < 10 ? 3 : -9);
    *y = yoe + era * 400 + (*m <= 2);
}

static int ltoa10(long v, char *out) {
    char t[16];
    int n = 0, k = 0;
    unsigned long u = v < 0 ? 0UL - (unsigned long)v : (unsigned long)v;
    do { t[n++] = (char)('0' + u % 10UL); u /= 10UL; } while (u);
    if (v < 0) out[k++] = '-';
    while (n) out[k++] = t[--n];
    return k;
}

int main(void) {
    /* what a 32-bit long means */
    {
        print_i64(sizeof(long));
        print_i64(LONG_MAX);
        print_i64(LONG_MIN);
        print_u64(ULONG_MAX);
        print_i64(KIND(2147483647));
        print_i64(KIND(2147483648));     /* too big for long: long long */
        print_i64(KIND(0xFFFFFFFF));     /* hexadecimal: unsigned int */
        print_i64(KIND(0xFFFFFFFFL));    /* unsigned long */
        print_i64(KIND(4294967296L));    /* long long */
        print_i64(KIND(1L + 1u));        /* long and unsigned int of one width: unsigned long */
        print_i64(KIND(1LL + 1UL));      /* long long holds every unsigned long */
        print_i64(-1L < 0u);             /* false: both become unsigned long */
        print_i64(-1LL < 0UL);           /* true */
        print_i64(sizeof(long long));
        print_i64(LONG_MAX == INT_MAX);
        unsigned long w = ULONG_MAX;
        w++;
        print_u64(w);
        long l = LONG_MAX;
        print_i64((long long)l + 1);
        print_i64(sizeof(int32_t) == sizeof(long) || sizeof(int32_t) == sizeof(int));
    }

    /* CRC-32 */
    {
        static unsigned char buf[1000];
        for (int i = 0; i < 256; i++) {
            unsigned long c = (unsigned long)i;
            for (int k = 0; k < 8; k++) c = c & 1UL ? 0xEDB88320UL ^ (c >> 1) : c >> 1;
            crc_table[i] = c;
        }
        for (int i = 0; i < 1000; i++) buf[i] = (unsigned char)(i * i + 17);
        unsigned long a = crc_bitwise(buf, 1000), b = crc_tabled(buf, 1000);
        print_u64(a);
        print_i64(a == b);
        print_u64(crc_tabled((const unsigned char *)"123456789", 9));
    }

    /* hashing and generators */
    {
        long total = 0;
        for (unsigned long k = 0; k < 2000UL; k++) total += bits(mix32(k) ^ mix32(k ^ 1UL));
        print_i64(total);
        unsigned long s = 0;
        for (int i = 0; i < 5000; i++) s += xorshift32() >> 4;
        print_u64(s);
        unsigned long lcg = 1;
        for (int i = 0; i < 100000; i++) lcg = lcg * 1664525UL + 1013904223UL;
        print_u64(lcg);
    }

    /* fixed point */
    {
        print_i64(fmul(FIX(3.5), FIX(-1.25)));
        print_i64(fdiv(FIX(1), FIX(7)));
        print_i64(fsqrt(FIX(2)));
        print_i64(fsqrt(FIX(10000)));
        fix acc = 0;
        for (int i = 1; i <= 100; i++) acc += fdiv(FIX(1), (fix)i * FIX(1));
        print_i64(acc);
    }

    /* calendars */
    {
        print_i64(days_from_civil(1970, 1, 1));
        print_i64(days_from_civil(2000, 2, 29));
        print_i64(days_from_civil(1600, 3, 1));
        print_i64(days_from_civil(-4713, 11, 24));
        long bad = 0, y, m, d;
        for (long z = -800000; z < 800000; z += 997) {
            civil_from_days(z, &y, &m, &d);
            if (days_from_civil(y, m, d) != z) bad++;
        }
        print_i64(bad);
        civil_from_days(20000, &y, &m, &d);
        print_i64(y * 10000 + m * 100 + d);
        char text[40];
        int n = ltoa10(LONG_MIN, text);
        print_str(text, (size_t)n);
        n = ltoa10(-1234567, text);
        print_str(text, (size_t)n);
    }
    return 0;
}

/* expect: 4 */
/* expect: 2147483647 */
/* expect: -2147483648 */
/* expect: 4294967295 */
/* expect: 1 */
/* expect: 5 */
/* expect: 2 */
/* expect: 4 */
/* expect: 5 */
/* expect: 4 */
/* expect: 5 */
/* expect: 0 */
/* expect: 1 */
/* expect: 8 */
/* expect: 1 */
/* expect: 0 */
/* expect: 2147483648 */
/* expect: 1 */
/* expect: 2578107429 */
/* expect: 1 */
/* expect: 3421780262 */
/* expect: 32260 */
/* expect: 1700368732 */
/* expect: 694140833 */
/* expect: -286720 */
/* expect: 9362 */
/* expect: 92681 */
/* expect: 6553600 */
/* expect: 339918 */
/* expect: 0 */
/* expect: 11016 */
/* expect: -135080 */
/* expect: -2440588 */
/* expect: 0 */
/* expect: 20241004 */
/* expect: -2147483648 */
/* expect: -1234567 */
