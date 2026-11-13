/* abi_avx.h - written by gen_c_abi.py 31 8: the types of the
   tests in abi_avx.c and abi_avx_callee.c, and the functions that fill
   and check them. */
#ifndef ABI_AVX_H
#define ABI_AVX_H
#include <stdarg.h>

extern char abi_anchor[8];
void abi_fail(int test, int what);

static unsigned long long abi_mix(unsigned s, unsigned j) {
    unsigned long long x = s * 0x9E3779B97F4A7C15ull + (j + 1u) * 0xBF58476D1CE4E5B9ull;
    x ^= x >> 29;
    x *= 0x94D049BB133111EBull;
    return x ^ (x >> 32);
}

static void abi_zero(void *p, unsigned long n) {
    unsigned char *q = (unsigned char *)p;
    while (n--) *q++ = 0;
}

typedef signed char abi_v4c __attribute__((vector_size(4)));
static void fill_abi_v4c(abi_v4c *p, unsigned s) {
    (*p)[0] = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p)[1] = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p)[2] = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p)[3] = (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
}
static int check_abi_v4c(const abi_v4c *p, unsigned s) {
    int bad = 0;
    bad |= (*p)[0] != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p)[1] != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p)[2] != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p)[3] != (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    return !bad;
}
typedef unsigned char abi_v8uc __attribute__((vector_size(8)));
static void fill_abi_v8uc(abi_v8uc *p, unsigned s) {
    (*p)[0] = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p)[1] = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p)[2] = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p)[3] = (unsigned char)(abi_mix(s, 3u) % 256u);
    (*p)[4] = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p)[5] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p)[6] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p)[7] = (unsigned char)(abi_mix(s, 7u) % 256u);
}
static int check_abi_v8uc(const abi_v8uc *p, unsigned s) {
    int bad = 0;
    bad |= (*p)[0] != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p)[1] != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p)[2] != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p)[3] != (unsigned char)(abi_mix(s, 3u) % 256u);
    bad |= (*p)[4] != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p)[5] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p)[6] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p)[7] != (unsigned char)(abi_mix(s, 7u) % 256u);
    return !bad;
}
typedef float abi_v2f __attribute__((vector_size(8)));
static void fill_abi_v2f(abi_v2f *p, unsigned s) {
    (*p)[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p)[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_abi_v2f(const abi_v2f *p, unsigned s) {
    int bad = 0;
    bad |= (*p)[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p)[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
typedef int abi_v4i __attribute__((vector_size(16)));
static void fill_abi_v4i(abi_v4i *p, unsigned s) {
    (*p)[0] = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p)[1] = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p)[2] = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p)[3] = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
}
static int check_abi_v4i(const abi_v4i *p, unsigned s) {
    int bad = 0;
    bad |= (*p)[0] != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p)[1] != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p)[2] != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p)[3] != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    return !bad;
}
typedef short abi_v8s __attribute__((vector_size(16)));
static void fill_abi_v8s(abi_v8s *p, unsigned s) {
    (*p)[0] = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p)[1] = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    (*p)[2] = (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    (*p)[3] = (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
    (*p)[4] = (short)((long long)(abi_mix(s, 4u) % 65536u) - 32768);
    (*p)[5] = (short)((long long)(abi_mix(s, 5u) % 65536u) - 32768);
    (*p)[6] = (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    (*p)[7] = (short)((long long)(abi_mix(s, 7u) % 65536u) - 32768);
}
static int check_abi_v8s(const abi_v8s *p, unsigned s) {
    int bad = 0;
    bad |= (*p)[0] != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p)[1] != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    bad |= (*p)[2] != (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    bad |= (*p)[3] != (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
    bad |= (*p)[4] != (short)((long long)(abi_mix(s, 4u) % 65536u) - 32768);
    bad |= (*p)[5] != (short)((long long)(abi_mix(s, 5u) % 65536u) - 32768);
    bad |= (*p)[6] != (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    bad |= (*p)[7] != (short)((long long)(abi_mix(s, 7u) % 65536u) - 32768);
    return !bad;
}
typedef float abi_v4f __attribute__((vector_size(16)));
static void fill_abi_v4f(abi_v4f *p, unsigned s) {
    (*p)[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p)[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p)[2] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p)[3] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_abi_v4f(const abi_v4f *p, unsigned s) {
    int bad = 0;
    bad |= (*p)[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p)[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p)[2] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p)[3] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
typedef double abi_v2d __attribute__((vector_size(16)));
static void fill_abi_v2d(abi_v2d *p, unsigned s) {
    (*p)[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p)[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_abi_v2d(const abi_v2d *p, unsigned s) {
    int bad = 0;
    bad |= (*p)[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p)[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
typedef long long abi_v2ll __attribute__((vector_size(16)));
static void fill_abi_v2ll(abi_v2ll *p, unsigned s) {
    (*p)[0] = (long long)abi_mix(s, 0u);
    (*p)[1] = (long long)abi_mix(s, 1u);
}
static int check_abi_v2ll(const abi_v2ll *p, unsigned s) {
    int bad = 0;
    bad |= (*p)[0] != (long long)abi_mix(s, 0u);
    bad |= (*p)[1] != (long long)abi_mix(s, 1u);
    return !bad;
}
typedef int abi_v8i __attribute__((vector_size(32)));
static void fill_abi_v8i(abi_v8i *p, unsigned s) {
    (*p)[0] = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p)[1] = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p)[2] = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p)[3] = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p)[4] = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p)[5] = (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    (*p)[6] = (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    (*p)[7] = (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
}
static int check_abi_v8i(const abi_v8i *p, unsigned s) {
    int bad = 0;
    bad |= (*p)[0] != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p)[1] != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p)[2] != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p)[3] != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p)[4] != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p)[5] != (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    bad |= (*p)[6] != (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    bad |= (*p)[7] != (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
    return !bad;
}
typedef double abi_v4d __attribute__((vector_size(32)));
static void fill_abi_v4d(abi_v4d *p, unsigned s) {
    (*p)[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p)[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p)[2] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p)[3] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_abi_v4d(const abi_v4d *p, unsigned s) {
    int bad = 0;
    bad |= (*p)[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p)[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p)[2] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p)[3] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
struct R0 {
    float m0;
};
struct R1 {
    abi_v2d m0;
};
struct R2 {
    abi_v2f m0;
    abi_v2f m1;
    abi_v2f m2;
};
struct R3 {
    abi_v2f m0;
    struct R2 m1;
    abi_v2f m2;
};
struct R4 {
    unsigned m0_0 : 29;
    int m0_1 : 22;
    int m0_2 : 21;
    unsigned m1_0 : 10;
    unsigned m1_1 : 12;
    unsigned m2_0 : 32;
    unsigned : 0;
    unsigned m2_1 : 22;
    unsigned m2_2 : 31;
};
struct R5 {
    abi_v8uc m0;
    unsigned : 0;
    unsigned m1_0 : 7;
    int m1_1 : 10;
    int : 3;
    int m1_2 : 12;
};
struct R6 {
    float m0;
    abi_v8uc m1[4];
};
struct R7 {
    abi_v2ll m0;
};
struct R8 {
    float m0;
    abi_v4f m1;
    abi_v4c m2;
    int m3;
};
struct R9 {
    abi_v4d m0;
    unsigned char m1[5];
    _Alignas(16) unsigned long long m2[2];
    struct R8 m3[2];
    _Bool m4_0 : 1;
    unsigned : 3;
    unsigned m4_1 : 22;
};
union R10 {
    char * m0;
    unsigned char m1;
};
struct R11 {
    short m0;
    int m1_0 : 3;
    unsigned m1_1 : 12;
    unsigned m1_2 : 1;
    int m1_3 : 3;
};
struct R12 {
    double m0;
};
struct R13 {
    short m0;
    char * m1;
    unsigned long long m2;
};
struct R14 {
    float m0;
    float m1;
    float m2;
};
struct R15 {
    float m0;
    struct R14 m1;
};
struct R16 {
    abi_v2f m0;
    abi_v2f m1;
    abi_v2f m2;
};
struct R17 {
    abi_v2f m0;
    struct R16 m1;
};
struct R18 {
    signed char m0;
    float m1;
};
struct R19 {
    abi_v2f m0;
    int m1_0 : 5;
    int m1_1 : 24;
    int m1_2 : 1;
    unsigned char m2[2];
};
struct R20 {
    struct R19 m0;
    unsigned m1_0 : 27;
    unsigned m1_1 : 13;
};
struct R21 {
    abi_v8uc m0;
    int m1;
};
struct R22 {
    abi_v4f m0;
};
struct R23 {
    abi_v2d m0;
};
struct R24 {
    abi_v2f m0;
    int m1_0 : 20;
    unsigned m1_1 : 2;
    unsigned m1_2 : 32;
};
struct R25 {
    int m0_0 : 10;
    int m0_1 : 4;
    int m0_2 : 29;
    unsigned m0_3 : 6;
    abi_v4c m1;
};
struct R26 {
    float m0;
    float m1;
};
struct R27 {
    double m0;
};
struct R28 {
    short m0;
};
union R29 {
    unsigned char m0[2];
    signed char m1;
};
struct R30 {
    abi_v4i m0;
    abi_v4i m1;
};
union R31 {
    abi_v8uc m0;
    abi_v8uc m1[3];
    abi_v2f m2;
    unsigned long long m3;
    unsigned long long m4[1];
    abi_v2ll m5;
};
struct R32 {
    abi_v4c m0;
    abi_v8s m1;
    abi_v2ll m2;
};
struct R33 {
    abi_v4f m0[1];
    abi_v8uc m1[2];
    _Bool m2;
    char * m3;
};
struct R34 {
    double m0;
};
struct R35 {
    abi_v4i m0;
    abi_v4i m1;
    abi_v4i m2;
};
struct R36 {
    abi_v4f m0;
};
struct R37 {
    unsigned long long m0[4];
    _Bool m1[1];
    unsigned char m2;
};
struct R38 {
    unsigned m0_0 : 16;
    unsigned m0_1 : 25;
    long long m1;
};
struct R39 {
    float m0;
    float m1;
};
union R40 {
    int m0;
    abi_v4d m1[2];
    unsigned short m2[2];
    char * m3[2];
    double m4[1];
};

static void fill_R0(struct R0 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R0(const struct R0 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R1(struct R1 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R1(const struct R1 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R2(struct R2 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m2[1] = (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
}
static int check_R2(const struct R2 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[1] != (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R3(struct R3 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0[0] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0[1] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1[0] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1[1] = (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2[0] = (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2[1] = (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    (*p).m2[1] = (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
}
static int check_R3(const struct R3 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0[0] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0[1] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1[0] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1[1] != (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2[0] != (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2[1] != (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[1] != (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R4(struct R4 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 536870912u);
    (*p).m0_1 = (int)((long long)(abi_mix(s, 1u) % 4194304u) - 2097152);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 2097152u) - 1048576);
    (*p).m1_0 = (unsigned)(abi_mix(s, 3u) % 1024u);
    (*p).m1_1 = (unsigned)(abi_mix(s, 4u) % 4096u);
    (*p).m2_0 = (unsigned)(abi_mix(s, 5u) % 4294967296u);
    (*p).m2_1 = (unsigned)(abi_mix(s, 6u) % 4194304u);
    (*p).m2_2 = (unsigned)(abi_mix(s, 7u) % 2147483648u);
}
static int check_R4(const struct R4 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 536870912u);
    bad |= (*p).m0_1 != (int)((long long)(abi_mix(s, 1u) % 4194304u) - 2097152);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 2097152u) - 1048576);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 3u) % 1024u);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 4u) % 4096u);
    bad |= (*p).m2_0 != (unsigned)(abi_mix(s, 5u) % 4294967296u);
    bad |= (*p).m2_1 != (unsigned)(abi_mix(s, 6u) % 4194304u);
    bad |= (*p).m2_2 != (unsigned)(abi_mix(s, 7u) % 2147483648u);
    return !bad;
}
static void fill_R5(struct R5 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m0[1] = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m0[2] = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p).m0[3] = (unsigned char)(abi_mix(s, 3u) % 256u);
    (*p).m0[4] = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p).m0[5] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m0[6] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m0[7] = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m1_0 = (unsigned)(abi_mix(s, 8u) % 128u);
    (*p).m1_1 = (int)((long long)(abi_mix(s, 9u) % 1024u) - 512);
    (*p).m1_2 = (int)((long long)(abi_mix(s, 10u) % 4096u) - 2048);
}
static int check_R5(const struct R5 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m0[1] != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m0[2] != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m0[3] != (unsigned char)(abi_mix(s, 3u) % 256u);
    bad |= (*p).m0[4] != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p).m0[5] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m0[6] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m0[7] != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 8u) % 128u);
    bad |= (*p).m1_1 != (int)((long long)(abi_mix(s, 9u) % 1024u) - 512);
    bad |= (*p).m1_2 != (int)((long long)(abi_mix(s, 10u) % 4096u) - 2048);
    return !bad;
}
static void fill_R6(struct R6 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0][0] = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m1[0][1] = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p).m1[0][2] = (unsigned char)(abi_mix(s, 3u) % 256u);
    (*p).m1[0][3] = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p).m1[0][4] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m1[0][5] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m1[0][6] = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m1[0][7] = (unsigned char)(abi_mix(s, 8u) % 256u);
    (*p).m1[1][0] = (unsigned char)(abi_mix(s, 9u) % 256u);
    (*p).m1[1][1] = (unsigned char)(abi_mix(s, 10u) % 256u);
    (*p).m1[1][2] = (unsigned char)(abi_mix(s, 11u) % 256u);
    (*p).m1[1][3] = (unsigned char)(abi_mix(s, 12u) % 256u);
    (*p).m1[1][4] = (unsigned char)(abi_mix(s, 13u) % 256u);
    (*p).m1[1][5] = (unsigned char)(abi_mix(s, 14u) % 256u);
    (*p).m1[1][6] = (unsigned char)(abi_mix(s, 15u) % 256u);
    (*p).m1[1][7] = (unsigned char)(abi_mix(s, 16u) % 256u);
    (*p).m1[2][0] = (unsigned char)(abi_mix(s, 17u) % 256u);
    (*p).m1[2][1] = (unsigned char)(abi_mix(s, 18u) % 256u);
    (*p).m1[2][2] = (unsigned char)(abi_mix(s, 19u) % 256u);
    (*p).m1[2][3] = (unsigned char)(abi_mix(s, 20u) % 256u);
    (*p).m1[2][4] = (unsigned char)(abi_mix(s, 21u) % 256u);
    (*p).m1[2][5] = (unsigned char)(abi_mix(s, 22u) % 256u);
    (*p).m1[2][6] = (unsigned char)(abi_mix(s, 23u) % 256u);
    (*p).m1[2][7] = (unsigned char)(abi_mix(s, 24u) % 256u);
    (*p).m1[3][0] = (unsigned char)(abi_mix(s, 25u) % 256u);
    (*p).m1[3][1] = (unsigned char)(abi_mix(s, 26u) % 256u);
    (*p).m1[3][2] = (unsigned char)(abi_mix(s, 27u) % 256u);
    (*p).m1[3][3] = (unsigned char)(abi_mix(s, 28u) % 256u);
    (*p).m1[3][4] = (unsigned char)(abi_mix(s, 29u) % 256u);
    (*p).m1[3][5] = (unsigned char)(abi_mix(s, 30u) % 256u);
    (*p).m1[3][6] = (unsigned char)(abi_mix(s, 31u) % 256u);
    (*p).m1[3][7] = (unsigned char)(abi_mix(s, 32u) % 256u);
}
static int check_R6(const struct R6 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0][0] != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m1[0][1] != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m1[0][2] != (unsigned char)(abi_mix(s, 3u) % 256u);
    bad |= (*p).m1[0][3] != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p).m1[0][4] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m1[0][5] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m1[0][6] != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m1[0][7] != (unsigned char)(abi_mix(s, 8u) % 256u);
    bad |= (*p).m1[1][0] != (unsigned char)(abi_mix(s, 9u) % 256u);
    bad |= (*p).m1[1][1] != (unsigned char)(abi_mix(s, 10u) % 256u);
    bad |= (*p).m1[1][2] != (unsigned char)(abi_mix(s, 11u) % 256u);
    bad |= (*p).m1[1][3] != (unsigned char)(abi_mix(s, 12u) % 256u);
    bad |= (*p).m1[1][4] != (unsigned char)(abi_mix(s, 13u) % 256u);
    bad |= (*p).m1[1][5] != (unsigned char)(abi_mix(s, 14u) % 256u);
    bad |= (*p).m1[1][6] != (unsigned char)(abi_mix(s, 15u) % 256u);
    bad |= (*p).m1[1][7] != (unsigned char)(abi_mix(s, 16u) % 256u);
    bad |= (*p).m1[2][0] != (unsigned char)(abi_mix(s, 17u) % 256u);
    bad |= (*p).m1[2][1] != (unsigned char)(abi_mix(s, 18u) % 256u);
    bad |= (*p).m1[2][2] != (unsigned char)(abi_mix(s, 19u) % 256u);
    bad |= (*p).m1[2][3] != (unsigned char)(abi_mix(s, 20u) % 256u);
    bad |= (*p).m1[2][4] != (unsigned char)(abi_mix(s, 21u) % 256u);
    bad |= (*p).m1[2][5] != (unsigned char)(abi_mix(s, 22u) % 256u);
    bad |= (*p).m1[2][6] != (unsigned char)(abi_mix(s, 23u) % 256u);
    bad |= (*p).m1[2][7] != (unsigned char)(abi_mix(s, 24u) % 256u);
    bad |= (*p).m1[3][0] != (unsigned char)(abi_mix(s, 25u) % 256u);
    bad |= (*p).m1[3][1] != (unsigned char)(abi_mix(s, 26u) % 256u);
    bad |= (*p).m1[3][2] != (unsigned char)(abi_mix(s, 27u) % 256u);
    bad |= (*p).m1[3][3] != (unsigned char)(abi_mix(s, 28u) % 256u);
    bad |= (*p).m1[3][4] != (unsigned char)(abi_mix(s, 29u) % 256u);
    bad |= (*p).m1[3][5] != (unsigned char)(abi_mix(s, 30u) % 256u);
    bad |= (*p).m1[3][6] != (unsigned char)(abi_mix(s, 31u) % 256u);
    bad |= (*p).m1[3][7] != (unsigned char)(abi_mix(s, 32u) % 256u);
    return !bad;
}
static void fill_R7(struct R7 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (long long)abi_mix(s, 0u);
    (*p).m0[1] = (long long)abi_mix(s, 1u);
}
static int check_R7(const struct R7 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (long long)abi_mix(s, 0u);
    bad |= (*p).m0[1] != (long long)abi_mix(s, 1u);
    return !bad;
}
static void fill_R8(struct R8 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1[2] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1[3] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    (*p).m2[1] = (signed char)((long long)(abi_mix(s, 6u) % 256u) - 128);
    (*p).m2[2] = (signed char)((long long)(abi_mix(s, 7u) % 256u) - 128);
    (*p).m2[3] = (signed char)((long long)(abi_mix(s, 8u) % 256u) - 128);
    (*p).m3 = (int)((long long)(abi_mix(s, 9u) % 4294967296u) - 2147483648);
}
static int check_R8(const struct R8 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[2] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[3] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    bad |= (*p).m2[1] != (signed char)((long long)(abi_mix(s, 6u) % 256u) - 128);
    bad |= (*p).m2[2] != (signed char)((long long)(abi_mix(s, 7u) % 256u) - 128);
    bad |= (*p).m2[3] != (signed char)((long long)(abi_mix(s, 8u) % 256u) - 128);
    bad |= (*p).m3 != (int)((long long)(abi_mix(s, 9u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R9(struct R9 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p).m1[1] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m1[2] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m1[3] = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m1[4] = (unsigned char)(abi_mix(s, 8u) % 256u);
    (*p).m2[0] = (unsigned long long)abi_mix(s, 9u);
    (*p).m2[1] = (unsigned long long)abi_mix(s, 10u);
    (*p).m3[0].m0 = (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    (*p).m3[0].m1[0] = (float)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    (*p).m3[0].m1[1] = (float)((double)(long long)(abi_mix(s, 13u) % 4001u) / 8.0 - 250.0);
    (*p).m3[0].m1[2] = (float)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    (*p).m3[0].m1[3] = (float)((double)(long long)(abi_mix(s, 15u) % 4001u) / 8.0 - 250.0);
    (*p).m3[0].m2[0] = (signed char)((long long)(abi_mix(s, 16u) % 256u) - 128);
    (*p).m3[0].m2[1] = (signed char)((long long)(abi_mix(s, 17u) % 256u) - 128);
    (*p).m3[0].m2[2] = (signed char)((long long)(abi_mix(s, 18u) % 256u) - 128);
    (*p).m3[0].m2[3] = (signed char)((long long)(abi_mix(s, 19u) % 256u) - 128);
    (*p).m3[0].m3 = (int)((long long)(abi_mix(s, 20u) % 4294967296u) - 2147483648);
    (*p).m3[1].m0 = (float)((double)(long long)(abi_mix(s, 21u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1].m1[0] = (float)((double)(long long)(abi_mix(s, 22u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1].m1[1] = (float)((double)(long long)(abi_mix(s, 23u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1].m1[2] = (float)((double)(long long)(abi_mix(s, 24u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1].m1[3] = (float)((double)(long long)(abi_mix(s, 25u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1].m2[0] = (signed char)((long long)(abi_mix(s, 26u) % 256u) - 128);
    (*p).m3[1].m2[1] = (signed char)((long long)(abi_mix(s, 27u) % 256u) - 128);
    (*p).m3[1].m2[2] = (signed char)((long long)(abi_mix(s, 28u) % 256u) - 128);
    (*p).m3[1].m2[3] = (signed char)((long long)(abi_mix(s, 29u) % 256u) - 128);
    (*p).m3[1].m3 = (int)((long long)(abi_mix(s, 30u) % 4294967296u) - 2147483648);
    (*p).m4_0 = (_Bool)(abi_mix(s, 31u) & 1u);
    (*p).m4_1 = (unsigned)(abi_mix(s, 32u) % 4194304u);
}
static int check_R9(const struct R9 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p).m1[1] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m1[2] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m1[3] != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m1[4] != (unsigned char)(abi_mix(s, 8u) % 256u);
    bad |= (*p).m2[0] != (unsigned long long)abi_mix(s, 9u);
    bad |= (*p).m2[1] != (unsigned long long)abi_mix(s, 10u);
    bad |= (*p).m3[0].m0 != (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[0].m1[0] != (float)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[0].m1[1] != (float)((double)(long long)(abi_mix(s, 13u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[0].m1[2] != (float)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[0].m1[3] != (float)((double)(long long)(abi_mix(s, 15u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[0].m2[0] != (signed char)((long long)(abi_mix(s, 16u) % 256u) - 128);
    bad |= (*p).m3[0].m2[1] != (signed char)((long long)(abi_mix(s, 17u) % 256u) - 128);
    bad |= (*p).m3[0].m2[2] != (signed char)((long long)(abi_mix(s, 18u) % 256u) - 128);
    bad |= (*p).m3[0].m2[3] != (signed char)((long long)(abi_mix(s, 19u) % 256u) - 128);
    bad |= (*p).m3[0].m3 != (int)((long long)(abi_mix(s, 20u) % 4294967296u) - 2147483648);
    bad |= (*p).m3[1].m0 != (float)((double)(long long)(abi_mix(s, 21u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1].m1[0] != (float)((double)(long long)(abi_mix(s, 22u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1].m1[1] != (float)((double)(long long)(abi_mix(s, 23u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1].m1[2] != (float)((double)(long long)(abi_mix(s, 24u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1].m1[3] != (float)((double)(long long)(abi_mix(s, 25u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1].m2[0] != (signed char)((long long)(abi_mix(s, 26u) % 256u) - 128);
    bad |= (*p).m3[1].m2[1] != (signed char)((long long)(abi_mix(s, 27u) % 256u) - 128);
    bad |= (*p).m3[1].m2[2] != (signed char)((long long)(abi_mix(s, 28u) % 256u) - 128);
    bad |= (*p).m3[1].m2[3] != (signed char)((long long)(abi_mix(s, 29u) % 256u) - 128);
    bad |= (*p).m3[1].m3 != (int)((long long)(abi_mix(s, 30u) % 4294967296u) - 2147483648);
    bad |= (*p).m4_0 != (_Bool)(abi_mix(s, 31u) & 1u);
    bad |= (*p).m4_1 != (unsigned)(abi_mix(s, 32u) % 4194304u);
    return !bad;
}
static void fill_R10(union R10 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
}
static int check_R10(const union R10 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    return !bad;
}
static void fill_R11(struct R11 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m1_0 = (int)((long long)(abi_mix(s, 1u) % 8u) - 4);
    (*p).m1_1 = (unsigned)(abi_mix(s, 2u) % 4096u);
    (*p).m1_2 = (unsigned)(abi_mix(s, 3u) & 1u);
    (*p).m1_3 = (int)((long long)(abi_mix(s, 4u) % 8u) - 4);
}
static int check_R11(const struct R11 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m1_0 != (int)((long long)(abi_mix(s, 1u) % 8u) - 4);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 2u) % 4096u);
    bad |= (*p).m1_2 != (unsigned)(abi_mix(s, 3u) & 1u);
    bad |= (*p).m1_3 != (int)((long long)(abi_mix(s, 4u) % 8u) - 4);
    return !bad;
}
static void fill_R12(struct R12 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R12(const struct R12 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R13(struct R13 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m1 = (abi_anchor + (abi_mix(s, 1u) % 8u));
    (*p).m2 = (unsigned long long)abi_mix(s, 2u);
}
static int check_R13(const struct R13 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m1 != (abi_anchor + (abi_mix(s, 1u) % 8u));
    bad |= (*p).m2 != (unsigned long long)abi_mix(s, 2u);
    return !bad;
}
static void fill_R14(struct R14 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R14(const struct R14 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R15(struct R15 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2 = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R15(const struct R15 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2 != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R16(struct R16 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m2[1] = (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
}
static int check_R16(const struct R16 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[1] != (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R17(struct R17 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0[0] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0[1] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1[0] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1[1] = (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2[0] = (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2[1] = (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
}
static int check_R17(const struct R17 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0[0] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0[1] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1[0] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1[1] != (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2[0] != (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2[1] != (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R18(struct R18 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R18(const struct R18 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R19(struct R19 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1_0 = (int)((long long)(abi_mix(s, 2u) % 32u) - 16);
    (*p).m1_1 = (int)((long long)(abi_mix(s, 3u) % 16777216u) - 8388608);
    (*p).m1_2 = (int)((long long)(abi_mix(s, 4u) % 2u) - 1);
    (*p).m2[0] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m2[1] = (unsigned char)(abi_mix(s, 6u) % 256u);
}
static int check_R19(const struct R19 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1_0 != (int)((long long)(abi_mix(s, 2u) % 32u) - 16);
    bad |= (*p).m1_1 != (int)((long long)(abi_mix(s, 3u) % 16777216u) - 8388608);
    bad |= (*p).m1_2 != (int)((long long)(abi_mix(s, 4u) % 2u) - 1);
    bad |= (*p).m2[0] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m2[1] != (unsigned char)(abi_mix(s, 6u) % 256u);
    return !bad;
}
static void fill_R20(struct R20 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1_0 = (int)((long long)(abi_mix(s, 2u) % 32u) - 16);
    (*p).m0.m1_1 = (int)((long long)(abi_mix(s, 3u) % 16777216u) - 8388608);
    (*p).m0.m1_2 = (int)((long long)(abi_mix(s, 4u) % 2u) - 1);
    (*p).m0.m2[0] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m0.m2[1] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m1_0 = (unsigned)(abi_mix(s, 7u) % 134217728u);
    (*p).m1_1 = (unsigned)(abi_mix(s, 8u) % 8192u);
}
static int check_R20(const struct R20 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1_0 != (int)((long long)(abi_mix(s, 2u) % 32u) - 16);
    bad |= (*p).m0.m1_1 != (int)((long long)(abi_mix(s, 3u) % 16777216u) - 8388608);
    bad |= (*p).m0.m1_2 != (int)((long long)(abi_mix(s, 4u) % 2u) - 1);
    bad |= (*p).m0.m2[0] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m0.m2[1] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 7u) % 134217728u);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 8u) % 8192u);
    return !bad;
}
static void fill_R21(struct R21 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m0[1] = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m0[2] = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p).m0[3] = (unsigned char)(abi_mix(s, 3u) % 256u);
    (*p).m0[4] = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p).m0[5] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m0[6] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m0[7] = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m1 = (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
}
static int check_R21(const struct R21 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m0[1] != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m0[2] != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m0[3] != (unsigned char)(abi_mix(s, 3u) % 256u);
    bad |= (*p).m0[4] != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p).m0[5] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m0[6] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m0[7] != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m1 != (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R22(struct R22 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R22(const struct R22 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R23(struct R23 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R23(const struct R23 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R24(struct R24 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1_0 = (int)((long long)(abi_mix(s, 2u) % 1048576u) - 524288);
    (*p).m1_1 = (unsigned)(abi_mix(s, 3u) % 4u);
    (*p).m1_2 = (unsigned)(abi_mix(s, 4u) % 4294967296u);
}
static int check_R24(const struct R24 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1_0 != (int)((long long)(abi_mix(s, 2u) % 1048576u) - 524288);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 3u) % 4u);
    bad |= (*p).m1_2 != (unsigned)(abi_mix(s, 4u) % 4294967296u);
    return !bad;
}
static void fill_R25(struct R25 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 1024u) - 512);
    (*p).m0_1 = (int)((long long)(abi_mix(s, 1u) % 16u) - 8);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 536870912u) - 268435456);
    (*p).m0_3 = (unsigned)(abi_mix(s, 3u) % 64u);
    (*p).m1[0] = (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
    (*p).m1[1] = (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    (*p).m1[2] = (signed char)((long long)(abi_mix(s, 6u) % 256u) - 128);
    (*p).m1[3] = (signed char)((long long)(abi_mix(s, 7u) % 256u) - 128);
}
static int check_R25(const struct R25 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 1024u) - 512);
    bad |= (*p).m0_1 != (int)((long long)(abi_mix(s, 1u) % 16u) - 8);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 536870912u) - 268435456);
    bad |= (*p).m0_3 != (unsigned)(abi_mix(s, 3u) % 64u);
    bad |= (*p).m1[0] != (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
    bad |= (*p).m1[1] != (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    bad |= (*p).m1[2] != (signed char)((long long)(abi_mix(s, 6u) % 256u) - 128);
    bad |= (*p).m1[3] != (signed char)((long long)(abi_mix(s, 7u) % 256u) - 128);
    return !bad;
}
static void fill_R26(struct R26 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R26(const struct R26 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R27(struct R27 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R27(const struct R27 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R28(struct R28 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
}
static int check_R28(const struct R28 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    return !bad;
}
static void fill_R29(union R29 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m0[1] = (unsigned char)(abi_mix(s, 1u) % 256u);
}
static int check_R29(const union R29 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m0[1] != (unsigned char)(abi_mix(s, 1u) % 256u);
    return !bad;
}
static void fill_R30(struct R30 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p).m0[1] = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m0[2] = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p).m0[3] = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p).m1[0] = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p).m1[1] = (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    (*p).m1[2] = (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    (*p).m1[3] = (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
}
static int check_R30(const struct R30 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[1] != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[2] != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[3] != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[0] != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[1] != (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[2] != (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[3] != (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R31(union R31 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m0[1] = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m0[2] = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p).m0[3] = (unsigned char)(abi_mix(s, 3u) % 256u);
    (*p).m0[4] = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p).m0[5] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m0[6] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m0[7] = (unsigned char)(abi_mix(s, 7u) % 256u);
}
static int check_R31(const union R31 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m0[1] != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m0[2] != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m0[3] != (unsigned char)(abi_mix(s, 3u) % 256u);
    bad |= (*p).m0[4] != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p).m0[5] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m0[6] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m0[7] != (unsigned char)(abi_mix(s, 7u) % 256u);
    return !bad;
}
static void fill_R32(struct R32 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0[1] = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m0[2] = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m0[3] = (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    (*p).m1[0] = (short)((long long)(abi_mix(s, 4u) % 65536u) - 32768);
    (*p).m1[1] = (short)((long long)(abi_mix(s, 5u) % 65536u) - 32768);
    (*p).m1[2] = (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    (*p).m1[3] = (short)((long long)(abi_mix(s, 7u) % 65536u) - 32768);
    (*p).m1[4] = (short)((long long)(abi_mix(s, 8u) % 65536u) - 32768);
    (*p).m1[5] = (short)((long long)(abi_mix(s, 9u) % 65536u) - 32768);
    (*p).m1[6] = (short)((long long)(abi_mix(s, 10u) % 65536u) - 32768);
    (*p).m1[7] = (short)((long long)(abi_mix(s, 11u) % 65536u) - 32768);
    (*p).m2[0] = (long long)abi_mix(s, 12u);
    (*p).m2[1] = (long long)abi_mix(s, 13u);
}
static int check_R32(const struct R32 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0[1] != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m0[2] != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m0[3] != (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    bad |= (*p).m1[0] != (short)((long long)(abi_mix(s, 4u) % 65536u) - 32768);
    bad |= (*p).m1[1] != (short)((long long)(abi_mix(s, 5u) % 65536u) - 32768);
    bad |= (*p).m1[2] != (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    bad |= (*p).m1[3] != (short)((long long)(abi_mix(s, 7u) % 65536u) - 32768);
    bad |= (*p).m1[4] != (short)((long long)(abi_mix(s, 8u) % 65536u) - 32768);
    bad |= (*p).m1[5] != (short)((long long)(abi_mix(s, 9u) % 65536u) - 32768);
    bad |= (*p).m1[6] != (short)((long long)(abi_mix(s, 10u) % 65536u) - 32768);
    bad |= (*p).m1[7] != (short)((long long)(abi_mix(s, 11u) % 65536u) - 32768);
    bad |= (*p).m2[0] != (long long)abi_mix(s, 12u);
    bad |= (*p).m2[1] != (long long)abi_mix(s, 13u);
    return !bad;
}
static void fill_R33(struct R33 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0][0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[0][1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[0][2] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[0][3] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0][0] = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p).m1[0][1] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m1[0][2] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m1[0][3] = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m1[0][4] = (unsigned char)(abi_mix(s, 8u) % 256u);
    (*p).m1[0][5] = (unsigned char)(abi_mix(s, 9u) % 256u);
    (*p).m1[0][6] = (unsigned char)(abi_mix(s, 10u) % 256u);
    (*p).m1[0][7] = (unsigned char)(abi_mix(s, 11u) % 256u);
    (*p).m1[1][0] = (unsigned char)(abi_mix(s, 12u) % 256u);
    (*p).m1[1][1] = (unsigned char)(abi_mix(s, 13u) % 256u);
    (*p).m1[1][2] = (unsigned char)(abi_mix(s, 14u) % 256u);
    (*p).m1[1][3] = (unsigned char)(abi_mix(s, 15u) % 256u);
    (*p).m1[1][4] = (unsigned char)(abi_mix(s, 16u) % 256u);
    (*p).m1[1][5] = (unsigned char)(abi_mix(s, 17u) % 256u);
    (*p).m1[1][6] = (unsigned char)(abi_mix(s, 18u) % 256u);
    (*p).m1[1][7] = (unsigned char)(abi_mix(s, 19u) % 256u);
    (*p).m2 = (_Bool)(abi_mix(s, 20u) & 1u);
    (*p).m3 = (abi_anchor + (abi_mix(s, 21u) % 8u));
}
static int check_R33(const struct R33 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0][0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[0][1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[0][2] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[0][3] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0][0] != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p).m1[0][1] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m1[0][2] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m1[0][3] != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m1[0][4] != (unsigned char)(abi_mix(s, 8u) % 256u);
    bad |= (*p).m1[0][5] != (unsigned char)(abi_mix(s, 9u) % 256u);
    bad |= (*p).m1[0][6] != (unsigned char)(abi_mix(s, 10u) % 256u);
    bad |= (*p).m1[0][7] != (unsigned char)(abi_mix(s, 11u) % 256u);
    bad |= (*p).m1[1][0] != (unsigned char)(abi_mix(s, 12u) % 256u);
    bad |= (*p).m1[1][1] != (unsigned char)(abi_mix(s, 13u) % 256u);
    bad |= (*p).m1[1][2] != (unsigned char)(abi_mix(s, 14u) % 256u);
    bad |= (*p).m1[1][3] != (unsigned char)(abi_mix(s, 15u) % 256u);
    bad |= (*p).m1[1][4] != (unsigned char)(abi_mix(s, 16u) % 256u);
    bad |= (*p).m1[1][5] != (unsigned char)(abi_mix(s, 17u) % 256u);
    bad |= (*p).m1[1][6] != (unsigned char)(abi_mix(s, 18u) % 256u);
    bad |= (*p).m1[1][7] != (unsigned char)(abi_mix(s, 19u) % 256u);
    bad |= (*p).m2 != (_Bool)(abi_mix(s, 20u) & 1u);
    bad |= (*p).m3 != (abi_anchor + (abi_mix(s, 21u) % 8u));
    return !bad;
}
static void fill_R34(struct R34 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R34(const struct R34 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R35(struct R35 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p).m0[1] = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m0[2] = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p).m0[3] = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p).m1[0] = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p).m1[1] = (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    (*p).m1[2] = (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    (*p).m1[3] = (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
    (*p).m2[0] = (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    (*p).m2[1] = (int)((long long)(abi_mix(s, 9u) % 4294967296u) - 2147483648);
    (*p).m2[2] = (int)((long long)(abi_mix(s, 10u) % 4294967296u) - 2147483648);
    (*p).m2[3] = (int)((long long)(abi_mix(s, 11u) % 4294967296u) - 2147483648);
}
static int check_R35(const struct R35 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[1] != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[2] != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[3] != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[0] != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[1] != (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[2] != (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[3] != (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[0] != (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[1] != (int)((long long)(abi_mix(s, 9u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[2] != (int)((long long)(abi_mix(s, 10u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[3] != (int)((long long)(abi_mix(s, 11u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R36(struct R36 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R36(const struct R36 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R37(struct R37 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned long long)abi_mix(s, 0u);
    (*p).m0[1] = (unsigned long long)abi_mix(s, 1u);
    (*p).m0[2] = (unsigned long long)abi_mix(s, 2u);
    (*p).m0[3] = (unsigned long long)abi_mix(s, 3u);
    (*p).m1[0] = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m2 = (unsigned char)(abi_mix(s, 5u) % 256u);
}
static int check_R37(const struct R37 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m0[1] != (unsigned long long)abi_mix(s, 1u);
    bad |= (*p).m0[2] != (unsigned long long)abi_mix(s, 2u);
    bad |= (*p).m0[3] != (unsigned long long)abi_mix(s, 3u);
    bad |= (*p).m1[0] != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m2 != (unsigned char)(abi_mix(s, 5u) % 256u);
    return !bad;
}
static void fill_R38(struct R38 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 65536u);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 33554432u);
    (*p).m1 = (long long)abi_mix(s, 2u);
}
static int check_R38(const struct R38 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 33554432u);
    bad |= (*p).m1 != (long long)abi_mix(s, 2u);
    return !bad;
}
static void fill_R39(struct R39 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R39(const struct R39 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R40(union R40 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
}
static int check_R40(const union R40 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    return !bad;
}

void abi_f0(double a0, abi_v2d a1, int a2, double a3, int a4, abi_v4i a5, double a6, unsigned char a7, struct R0 a8, ...);
void abi_f1(struct R4 a0, unsigned short a1, struct R5 a2);
struct R6 abi_f2(int a0, ...);
struct R12 abi_f3(abi_v2d a0, unsigned short a1, struct R13 a2, abi_v4i a3, struct R15 a4, struct R17 a5);
struct R18 abi_f4(double a0, int a1, unsigned a2, struct R20 a3, signed char a4, struct R21 a5, struct R22 a6, struct R23 a7, abi_v8i a8, struct R24 a9, abi_v4c a10, abi_v8uc a11);
struct R25 abi_f5(abi_v4i a0, short a1, struct R26 a2, struct R27 a3, struct R28 a4, char * a5, union R29 a6, struct R30 a7);
union R31 abi_f6(_Bool a0, struct R32 a1, _Bool a2, abi_v4f a3, unsigned char a4, struct R33 a5, struct R34 a6, abi_v4c a7, struct R35 a8, struct R36 a9);
struct R37 abi_f7(float a0, float a1, unsigned long long a2, ...);
#endif
