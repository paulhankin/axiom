/* abi_gen_106.h - written by gen_c_abi.py 105 8: the types of the
   tests in abi_gen_106.c and abi_gen_106_callee.c, and the functions that fill
   and check them. */
#ifndef ABI_GEN_106_H
#define ABI_GEN_106_H
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
struct R0 {
    char * m0[2];
    _Bool m1_0 : 1;
    unsigned m1_1 : 25;
    unsigned m1_2 : 15;
    unsigned m1_3 : 20;
    float m2;
};
struct R1 {
    _Bool m0;
    unsigned long long m1;
    short m2;
};
struct R2 {
    abi_v4c m0;
    struct R1 m1;
};
struct R3 {
    struct R2 m0;
};
struct R4 {
    signed char m0;
};
struct R5 {
    unsigned m0_0 : 5;
    int m0_1 : 31;
    int m0_2 : 26;
    int m0_3 : 23;
    struct R4 m1;
    abi_v8uc m2[4];
    abi_v4c m3;
};
union R6 {
    abi_v8uc m0;
    abi_v4c m1;
    _Float128 m2;
    unsigned m3;
    abi_v4f m4;
};
struct R7 {
    abi_v2d m0[5];
    union R6 m1;
    _Alignas(32) unsigned short m2;
    _Bool m3_0 : 1;
    _Bool m4_0 : 1;
    unsigned m4_1 : 1;
};
struct R8 {
    _Alignas(16) abi_v4f m0;
    unsigned short m1;
    abi_v2ll m2;
    short m3;
};
struct R9 {
    char * m0;
    char * m1[3];
    _Bool m2_0 : 1;
    unsigned m2_1 : 11;
    unsigned m2_2 : 22;
    _Bool m2_3 : 1;
    struct R8 m3;
    unsigned m4_0 : 32;
    unsigned m4_1 : 30;
};
struct R10 {
    char * m0;
    unsigned m1_0 : 31;
    int m1_1 : 14;
    unsigned m1_2 : 21;
    unsigned m2_0 : 5;
    struct R7 m3;
    _Bool m4;
    _Alignas(64) struct R9 m5;
};
struct R11 {
    abi_v2f m0;
    abi_v2f m1;
};
struct R12 {
    abi_v2f m0[1];
    struct R11 m1;
    abi_v2f m2;
    abi_v2f m3;
};
struct R13 {
    unsigned long long m0;
    float m1[1];
    abi_v2f m2;
    _Alignas(64) abi_v2ll m3;
    signed char m4;
};
struct R14 {
    double m0;
    double m1[4];
};
struct R15 {
    abi_v2f m0;
    abi_v2f m1;
    abi_v2f m2;
};
struct R16 {
    float m0;
    _Alignas(16) abi_v2f m1;
    abi_v4i m2[5];
    int m3_0 : 5;
    _Bool m3_1 : 1;
};
union R17 {
    unsigned short m0[5];
    unsigned long long m1;
};
struct R18 {
    short m0;
    short m1;
    abi_v2ll m2[2];
};
struct R19 {
    int m0_0 : 25;
    _Bool m0_1 : 1;
    int m0_2 : 3;
    int m0_3 : 5;
};
struct R20 {
    _Alignas(32) struct R19 m0;
    unsigned m1_0 : 7;
    int m1_1 : 17;
    _Alignas(64) long long m2;
};
struct R21 {
    signed char m0;
};
struct R22 {
    struct R21 m0;
    abi_v2f m1;
    _Bool m2_0 : 1;
    _Bool m2_1 : 1;
    abi_v8uc m3;
};
struct R23 {
    unsigned short m0;
    abi_v8uc m1;
};
struct R24 {
    struct R20 m0;
    _Bool m1_0 : 1;
    unsigned m1_1 : 32;
    struct R22 m2;
    int m3_0 : 15;
    struct R23 m4;
    _Alignas(64) char * m5;
    abi_v8uc m6;
    unsigned short m7;
};
struct R25 {
    abi_v2d m0;
    unsigned long long m1;
    short m2;
};
struct R26 {
    _Float16 m0;
    _Float16 m1;
};
struct R27 {
    struct R26 m0;
};
struct R28 {
    short m0[1];
};
struct R29 {
    abi_v2f m0;
    float m1;
};
union R30 {
    struct R29 m0;
    abi_v4c m1;
};
struct R31 {
    double m0;
    union R30 m1;
};
union R32 {
    signed char m0;
};
struct R33 {
    int m0_0 : 10;
    int m0_1 : 27;
    abi_v2d m1;
    unsigned short m2;
};
struct R34 {
    struct R33 m0;
};
struct R35 {
    unsigned short m0;
};
struct R36 {
    abi_v2f m0;
    long long m1[5];
    signed char m2[2];
    _Float16 m3;
    struct R35 m4;
};
struct R37 {
    abi_v4f m0;
};
struct R38 {
    float m0;
};
struct R39 {
    struct R38 m0;
    float m1;
    float m2;
};
union R40 {
    int m0;
    long long m1;
    double m2;
};
struct R41 {
    unsigned m0_0 : 18;
    unsigned m0_1 : 25;
    unsigned : 0;
    unsigned m0_2 : 31;
    _Bool m0_3 : 1;
    unsigned short m1;
};
struct R42 {
    _Alignas(16) _Float16 m0;
    long long m1;
    abi_v4c m2;
    _Alignas(8) float m3;
    struct R41 m4;
    abi_v4c m5;
};
struct R43 {
    abi_v2f m0;
};
struct R44 {
    double m0;
    double m1;
    double m2;
    double m3;
    double m4;
};
union R45 {
    unsigned char m0;
    unsigned m1;
};
struct R46 {
    union R45 m0;
};
struct R47 {
    _Float16 m0[1];
    struct R46 m1;
};
struct R48 {
    int m0_0 : 14;
    unsigned m0_1 : 12;
    unsigned m0_2 : 28;
    int m0_3 : 12;
};
struct R49 {
    _Bool m0;
    abi_v2f m1;
    float m2;
    unsigned : 0;
    _Bool m3_0 : 1;
    int m3_1 : 22;
    int m3_2 : 19;
    unsigned m3_3 : 16;
    long long m4;
};
struct R50 {
    double m0;
    long long m1;
};
struct R51 {
    signed char m0;
    double m1;
    unsigned m2;
    abi_v2f m3;
};
union R52 {
    _Alignas(64) int m0;
    float m1;
};
struct R53 {
    _Bool m0_0 : 1;
    _Bool m0_1 : 1;
    int m0_2 : 27;
    int m1_0 : 29;
    long long m2[2];
    int m3_0 : 5;
    int m4_0 : 25;
    unsigned m4_1 : 9;
    unsigned m4_2 : 14;
    unsigned m4_3 : 20;
    unsigned long long m5[1];
    signed char m6;
    union R52 m7;
};
struct R54 {
    abi_v4i m0;
};
struct R55 {
    abi_v2d m0;
    _Bool m1[4];
};
struct R56 {
    abi_v4c m0;
};
struct R57 {
    unsigned char m0;
    abi_v2f m1;
    abi_v2d m2;
};
union R58 {
    abi_v2ll m0;
    abi_v2ll m1;
    struct R57 m2;
    abi_v4c m3;
    _Alignas(64) char * m4;
    int m5;
    unsigned long long m6;
    char * m7;
};
struct R59 {
    unsigned char m0;
    struct R51 m1;
    struct R53 m2[3];
    struct R54 m3;
    struct R55 m4;
    _Alignas(32) struct R56 m5;
    union R58 m6;
    _Bool m7_0 : 1;
    int m7_1 : 1;
    _Bool m7_2 : 1;
};
struct R60 {
    abi_v4f m0;
};
struct R61 {
    abi_v2f m0[4];
};
union R62 {
    int m0;
    abi_v4f m1;
};
struct R63 {
    _Bool m0;
    unsigned char m1;
};
union R64 {
    unsigned char m0;
    double m1;
};
union R65 {
    unsigned short m0;
    float m1;
    unsigned char m2;
    struct R63 m3;
    abi_v2ll m4;
    union R64 m5;
};
struct R66 {
    signed char m0[2];
    union R65 m1;
    unsigned short m2;
};
struct R67 {
    unsigned m0;
    long long m1;
    abi_v2ll m2[4];
};
struct R68 {
    abi_v8uc m0;
    unsigned char m1;
    char * m2;
    abi_v2f m3;
};
struct R69 {
    long long m0;
    abi_v4i m1;
    double m2;
};
struct R70 {
    struct R69 m0;
};
struct R71 {
    _Float16 m0;
    struct R67 m1;
    _Alignas(16) int m2;
    struct R68 m3;
    unsigned m4;
    _Alignas(16) struct R70 m5;
};
struct R72 {
    _Float128 m0;
    _Float128 m1;
};
struct R73 {
    struct R72 m0;
};
struct R74 {
    char * m0;
    unsigned long long m1[3];
};
struct R75 {
    _Float16 m0;
    _Float16 m1;
};
struct R76 {
    struct R75 m0;
    _Float16 m1;
};
struct R77 {
    _Alignas(32) short m0[1];
    abi_v4i m1[1];
    abi_v8s m2;
};
struct R78 {
    double m0;
    double m1;
    double m2;
};
struct R79 {
    struct R78 m0;
    double m1;
};
struct R80 {
    int m0;
};

static void fill_R0(struct R0 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (abi_anchor + (abi_mix(s, 0u) % 8u));
    (*p).m0[1] = (abi_anchor + (abi_mix(s, 1u) % 8u));
    (*p).m1_0 = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m1_1 = (unsigned)(abi_mix(s, 3u) % 33554432u);
    (*p).m1_2 = (unsigned)(abi_mix(s, 4u) % 32768u);
    (*p).m1_3 = (unsigned)(abi_mix(s, 5u) % 1048576u);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
}
static int check_R0(const struct R0 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (abi_anchor + (abi_mix(s, 0u) % 8u));
    bad |= (*p).m0[1] != (abi_anchor + (abi_mix(s, 1u) % 8u));
    bad |= (*p).m1_0 != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 3u) % 33554432u);
    bad |= (*p).m1_2 != (unsigned)(abi_mix(s, 4u) % 32768u);
    bad |= (*p).m1_3 != (unsigned)(abi_mix(s, 5u) % 1048576u);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R1(struct R1 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m1 = (unsigned long long)abi_mix(s, 1u);
    (*p).m2 = (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
}
static int check_R1(const struct R1 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m1 != (unsigned long long)abi_mix(s, 1u);
    bad |= (*p).m2 != (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    return !bad;
}
static void fill_R2(struct R2 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0[1] = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m0[2] = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m0[3] = (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    (*p).m1.m0 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m1.m1 = (unsigned long long)abi_mix(s, 5u);
    (*p).m1.m2 = (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
}
static int check_R2(const struct R2 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0[1] != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m0[2] != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m0[3] != (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    bad |= (*p).m1.m0 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m1.m1 != (unsigned long long)abi_mix(s, 5u);
    bad |= (*p).m1.m2 != (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    return !bad;
}
static void fill_R3(struct R3 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0[0] = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0.m0[1] = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m0.m0[2] = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m0.m0[3] = (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    (*p).m0.m1.m0 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m0.m1.m1 = (unsigned long long)abi_mix(s, 5u);
    (*p).m0.m1.m2 = (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
}
static int check_R3(const struct R3 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0[0] != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0.m0[1] != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m0.m0[2] != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m0.m0[3] != (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    bad |= (*p).m0.m1.m0 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m0.m1.m1 != (unsigned long long)abi_mix(s, 5u);
    bad |= (*p).m0.m1.m2 != (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    return !bad;
}
static void fill_R4(struct R4 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
}
static int check_R4(const struct R4 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    return !bad;
}
static void fill_R5(struct R5 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 32u);
    (*p).m0_1 = (int)((long long)(abi_mix(s, 1u) % 2147483648u) - 1073741824);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 67108864u) - 33554432);
    (*p).m0_3 = (int)((long long)(abi_mix(s, 3u) % 8388608u) - 4194304);
    (*p).m1.m0 = (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
    (*p).m2[0][0] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m2[0][1] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m2[0][2] = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m2[0][3] = (unsigned char)(abi_mix(s, 8u) % 256u);
    (*p).m2[0][4] = (unsigned char)(abi_mix(s, 9u) % 256u);
    (*p).m2[0][5] = (unsigned char)(abi_mix(s, 10u) % 256u);
    (*p).m2[0][6] = (unsigned char)(abi_mix(s, 11u) % 256u);
    (*p).m2[0][7] = (unsigned char)(abi_mix(s, 12u) % 256u);
    (*p).m2[1][0] = (unsigned char)(abi_mix(s, 13u) % 256u);
    (*p).m2[1][1] = (unsigned char)(abi_mix(s, 14u) % 256u);
    (*p).m2[1][2] = (unsigned char)(abi_mix(s, 15u) % 256u);
    (*p).m2[1][3] = (unsigned char)(abi_mix(s, 16u) % 256u);
    (*p).m2[1][4] = (unsigned char)(abi_mix(s, 17u) % 256u);
    (*p).m2[1][5] = (unsigned char)(abi_mix(s, 18u) % 256u);
    (*p).m2[1][6] = (unsigned char)(abi_mix(s, 19u) % 256u);
    (*p).m2[1][7] = (unsigned char)(abi_mix(s, 20u) % 256u);
    (*p).m2[2][0] = (unsigned char)(abi_mix(s, 21u) % 256u);
    (*p).m2[2][1] = (unsigned char)(abi_mix(s, 22u) % 256u);
    (*p).m2[2][2] = (unsigned char)(abi_mix(s, 23u) % 256u);
    (*p).m2[2][3] = (unsigned char)(abi_mix(s, 24u) % 256u);
    (*p).m2[2][4] = (unsigned char)(abi_mix(s, 25u) % 256u);
    (*p).m2[2][5] = (unsigned char)(abi_mix(s, 26u) % 256u);
    (*p).m2[2][6] = (unsigned char)(abi_mix(s, 27u) % 256u);
    (*p).m2[2][7] = (unsigned char)(abi_mix(s, 28u) % 256u);
    (*p).m2[3][0] = (unsigned char)(abi_mix(s, 29u) % 256u);
    (*p).m2[3][1] = (unsigned char)(abi_mix(s, 30u) % 256u);
    (*p).m2[3][2] = (unsigned char)(abi_mix(s, 31u) % 256u);
    (*p).m2[3][3] = (unsigned char)(abi_mix(s, 32u) % 256u);
    (*p).m2[3][4] = (unsigned char)(abi_mix(s, 33u) % 256u);
    (*p).m2[3][5] = (unsigned char)(abi_mix(s, 34u) % 256u);
    (*p).m2[3][6] = (unsigned char)(abi_mix(s, 35u) % 256u);
    (*p).m2[3][7] = (unsigned char)(abi_mix(s, 36u) % 256u);
    (*p).m3[0] = (signed char)((long long)(abi_mix(s, 37u) % 256u) - 128);
    (*p).m3[1] = (signed char)((long long)(abi_mix(s, 38u) % 256u) - 128);
    (*p).m3[2] = (signed char)((long long)(abi_mix(s, 39u) % 256u) - 128);
    (*p).m3[3] = (signed char)((long long)(abi_mix(s, 40u) % 256u) - 128);
}
static int check_R5(const struct R5 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 32u);
    bad |= (*p).m0_1 != (int)((long long)(abi_mix(s, 1u) % 2147483648u) - 1073741824);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 67108864u) - 33554432);
    bad |= (*p).m0_3 != (int)((long long)(abi_mix(s, 3u) % 8388608u) - 4194304);
    bad |= (*p).m1.m0 != (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
    bad |= (*p).m2[0][0] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m2[0][1] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m2[0][2] != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m2[0][3] != (unsigned char)(abi_mix(s, 8u) % 256u);
    bad |= (*p).m2[0][4] != (unsigned char)(abi_mix(s, 9u) % 256u);
    bad |= (*p).m2[0][5] != (unsigned char)(abi_mix(s, 10u) % 256u);
    bad |= (*p).m2[0][6] != (unsigned char)(abi_mix(s, 11u) % 256u);
    bad |= (*p).m2[0][7] != (unsigned char)(abi_mix(s, 12u) % 256u);
    bad |= (*p).m2[1][0] != (unsigned char)(abi_mix(s, 13u) % 256u);
    bad |= (*p).m2[1][1] != (unsigned char)(abi_mix(s, 14u) % 256u);
    bad |= (*p).m2[1][2] != (unsigned char)(abi_mix(s, 15u) % 256u);
    bad |= (*p).m2[1][3] != (unsigned char)(abi_mix(s, 16u) % 256u);
    bad |= (*p).m2[1][4] != (unsigned char)(abi_mix(s, 17u) % 256u);
    bad |= (*p).m2[1][5] != (unsigned char)(abi_mix(s, 18u) % 256u);
    bad |= (*p).m2[1][6] != (unsigned char)(abi_mix(s, 19u) % 256u);
    bad |= (*p).m2[1][7] != (unsigned char)(abi_mix(s, 20u) % 256u);
    bad |= (*p).m2[2][0] != (unsigned char)(abi_mix(s, 21u) % 256u);
    bad |= (*p).m2[2][1] != (unsigned char)(abi_mix(s, 22u) % 256u);
    bad |= (*p).m2[2][2] != (unsigned char)(abi_mix(s, 23u) % 256u);
    bad |= (*p).m2[2][3] != (unsigned char)(abi_mix(s, 24u) % 256u);
    bad |= (*p).m2[2][4] != (unsigned char)(abi_mix(s, 25u) % 256u);
    bad |= (*p).m2[2][5] != (unsigned char)(abi_mix(s, 26u) % 256u);
    bad |= (*p).m2[2][6] != (unsigned char)(abi_mix(s, 27u) % 256u);
    bad |= (*p).m2[2][7] != (unsigned char)(abi_mix(s, 28u) % 256u);
    bad |= (*p).m2[3][0] != (unsigned char)(abi_mix(s, 29u) % 256u);
    bad |= (*p).m2[3][1] != (unsigned char)(abi_mix(s, 30u) % 256u);
    bad |= (*p).m2[3][2] != (unsigned char)(abi_mix(s, 31u) % 256u);
    bad |= (*p).m2[3][3] != (unsigned char)(abi_mix(s, 32u) % 256u);
    bad |= (*p).m2[3][4] != (unsigned char)(abi_mix(s, 33u) % 256u);
    bad |= (*p).m2[3][5] != (unsigned char)(abi_mix(s, 34u) % 256u);
    bad |= (*p).m2[3][6] != (unsigned char)(abi_mix(s, 35u) % 256u);
    bad |= (*p).m2[3][7] != (unsigned char)(abi_mix(s, 36u) % 256u);
    bad |= (*p).m3[0] != (signed char)((long long)(abi_mix(s, 37u) % 256u) - 128);
    bad |= (*p).m3[1] != (signed char)((long long)(abi_mix(s, 38u) % 256u) - 128);
    bad |= (*p).m3[2] != (signed char)((long long)(abi_mix(s, 39u) % 256u) - 128);
    bad |= (*p).m3[3] != (signed char)((long long)(abi_mix(s, 40u) % 256u) - 128);
    return !bad;
}
static void fill_R6(union R6 *p, unsigned s) {
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
static int check_R6(const union R6 *p, unsigned s) {
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
static void fill_R7(struct R7 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0][0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[0][1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1][0] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1][1] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2][0] = (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2][1] = (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3][0] = (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3][1] = (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m0[4][0] = (double)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    (*p).m0[4][1] = (double)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0[0] = (unsigned char)(abi_mix(s, 10u) % 256u);
    (*p).m1.m0[1] = (unsigned char)(abi_mix(s, 11u) % 256u);
    (*p).m1.m0[2] = (unsigned char)(abi_mix(s, 12u) % 256u);
    (*p).m1.m0[3] = (unsigned char)(abi_mix(s, 13u) % 256u);
    (*p).m1.m0[4] = (unsigned char)(abi_mix(s, 14u) % 256u);
    (*p).m1.m0[5] = (unsigned char)(abi_mix(s, 15u) % 256u);
    (*p).m1.m0[6] = (unsigned char)(abi_mix(s, 16u) % 256u);
    (*p).m1.m0[7] = (unsigned char)(abi_mix(s, 17u) % 256u);
    (*p).m2 = (unsigned short)(abi_mix(s, 18u) % 65536u);
    (*p).m3_0 = (_Bool)(abi_mix(s, 19u) & 1u);
    (*p).m4_0 = (_Bool)(abi_mix(s, 20u) & 1u);
    (*p).m4_1 = (unsigned)(abi_mix(s, 21u) & 1u);
}
static int check_R7(const struct R7 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0][0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[0][1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1][0] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1][1] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2][0] != (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2][1] != (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3][0] != (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3][1] != (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[4][0] != (double)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[4][1] != (double)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0[0] != (unsigned char)(abi_mix(s, 10u) % 256u);
    bad |= (*p).m1.m0[1] != (unsigned char)(abi_mix(s, 11u) % 256u);
    bad |= (*p).m1.m0[2] != (unsigned char)(abi_mix(s, 12u) % 256u);
    bad |= (*p).m1.m0[3] != (unsigned char)(abi_mix(s, 13u) % 256u);
    bad |= (*p).m1.m0[4] != (unsigned char)(abi_mix(s, 14u) % 256u);
    bad |= (*p).m1.m0[5] != (unsigned char)(abi_mix(s, 15u) % 256u);
    bad |= (*p).m1.m0[6] != (unsigned char)(abi_mix(s, 16u) % 256u);
    bad |= (*p).m1.m0[7] != (unsigned char)(abi_mix(s, 17u) % 256u);
    bad |= (*p).m2 != (unsigned short)(abi_mix(s, 18u) % 65536u);
    bad |= (*p).m3_0 != (_Bool)(abi_mix(s, 19u) & 1u);
    bad |= (*p).m4_0 != (_Bool)(abi_mix(s, 20u) & 1u);
    bad |= (*p).m4_1 != (unsigned)(abi_mix(s, 21u) & 1u);
    return !bad;
}
static void fill_R8(struct R8 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (unsigned short)(abi_mix(s, 4u) % 65536u);
    (*p).m2[0] = (long long)abi_mix(s, 5u);
    (*p).m2[1] = (long long)abi_mix(s, 6u);
    (*p).m3 = (short)((long long)(abi_mix(s, 7u) % 65536u) - 32768);
}
static int check_R8(const struct R8 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (unsigned short)(abi_mix(s, 4u) % 65536u);
    bad |= (*p).m2[0] != (long long)abi_mix(s, 5u);
    bad |= (*p).m2[1] != (long long)abi_mix(s, 6u);
    bad |= (*p).m3 != (short)((long long)(abi_mix(s, 7u) % 65536u) - 32768);
    return !bad;
}
static void fill_R9(struct R9 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
    (*p).m1[0] = (abi_anchor + (abi_mix(s, 1u) % 8u));
    (*p).m1[1] = (abi_anchor + (abi_mix(s, 2u) % 8u));
    (*p).m1[2] = (abi_anchor + (abi_mix(s, 3u) % 8u));
    (*p).m2_0 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m2_1 = (unsigned)(abi_mix(s, 5u) % 2048u);
    (*p).m2_2 = (unsigned)(abi_mix(s, 6u) % 4194304u);
    (*p).m2_3 = (_Bool)(abi_mix(s, 7u) & 1u);
    (*p).m3.m0[0] = (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[1] = (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[2] = (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[3] = (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m1 = (unsigned short)(abi_mix(s, 12u) % 65536u);
    (*p).m3.m2[0] = (long long)abi_mix(s, 13u);
    (*p).m3.m2[1] = (long long)abi_mix(s, 14u);
    (*p).m3.m3 = (short)((long long)(abi_mix(s, 15u) % 65536u) - 32768);
    (*p).m4_0 = (unsigned)(abi_mix(s, 16u) % 4294967296u);
    (*p).m4_1 = (unsigned)(abi_mix(s, 17u) % 1073741824u);
}
static int check_R9(const struct R9 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    bad |= (*p).m1[0] != (abi_anchor + (abi_mix(s, 1u) % 8u));
    bad |= (*p).m1[1] != (abi_anchor + (abi_mix(s, 2u) % 8u));
    bad |= (*p).m1[2] != (abi_anchor + (abi_mix(s, 3u) % 8u));
    bad |= (*p).m2_0 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m2_1 != (unsigned)(abi_mix(s, 5u) % 2048u);
    bad |= (*p).m2_2 != (unsigned)(abi_mix(s, 6u) % 4194304u);
    bad |= (*p).m2_3 != (_Bool)(abi_mix(s, 7u) & 1u);
    bad |= (*p).m3.m0[0] != (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[1] != (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[2] != (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[3] != (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m1 != (unsigned short)(abi_mix(s, 12u) % 65536u);
    bad |= (*p).m3.m2[0] != (long long)abi_mix(s, 13u);
    bad |= (*p).m3.m2[1] != (long long)abi_mix(s, 14u);
    bad |= (*p).m3.m3 != (short)((long long)(abi_mix(s, 15u) % 65536u) - 32768);
    bad |= (*p).m4_0 != (unsigned)(abi_mix(s, 16u) % 4294967296u);
    bad |= (*p).m4_1 != (unsigned)(abi_mix(s, 17u) % 1073741824u);
    return !bad;
}
static void fill_R10(struct R10 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
    (*p).m1_0 = (unsigned)(abi_mix(s, 1u) % 2147483648u);
    (*p).m1_1 = (int)((long long)(abi_mix(s, 2u) % 16384u) - 8192);
    (*p).m1_2 = (unsigned)(abi_mix(s, 3u) % 2097152u);
    (*p).m2_0 = (unsigned)(abi_mix(s, 4u) % 32u);
    (*p).m3.m0[0][0] = (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[0][1] = (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[1][0] = (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[1][1] = (double)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[2][0] = (double)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[2][1] = (double)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[3][0] = (double)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[3][1] = (double)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[4][0] = (double)((double)(long long)(abi_mix(s, 13u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0[4][1] = (double)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m1.m0[0] = (unsigned char)(abi_mix(s, 15u) % 256u);
    (*p).m3.m1.m0[1] = (unsigned char)(abi_mix(s, 16u) % 256u);
    (*p).m3.m1.m0[2] = (unsigned char)(abi_mix(s, 17u) % 256u);
    (*p).m3.m1.m0[3] = (unsigned char)(abi_mix(s, 18u) % 256u);
    (*p).m3.m1.m0[4] = (unsigned char)(abi_mix(s, 19u) % 256u);
    (*p).m3.m1.m0[5] = (unsigned char)(abi_mix(s, 20u) % 256u);
    (*p).m3.m1.m0[6] = (unsigned char)(abi_mix(s, 21u) % 256u);
    (*p).m3.m1.m0[7] = (unsigned char)(abi_mix(s, 22u) % 256u);
    (*p).m3.m2 = (unsigned short)(abi_mix(s, 23u) % 65536u);
    (*p).m3.m3_0 = (_Bool)(abi_mix(s, 24u) & 1u);
    (*p).m3.m4_0 = (_Bool)(abi_mix(s, 25u) & 1u);
    (*p).m3.m4_1 = (unsigned)(abi_mix(s, 26u) & 1u);
    (*p).m4 = (_Bool)(abi_mix(s, 27u) & 1u);
    (*p).m5.m0 = (abi_anchor + (abi_mix(s, 28u) % 8u));
    (*p).m5.m1[0] = (abi_anchor + (abi_mix(s, 29u) % 8u));
    (*p).m5.m1[1] = (abi_anchor + (abi_mix(s, 30u) % 8u));
    (*p).m5.m1[2] = (abi_anchor + (abi_mix(s, 31u) % 8u));
    (*p).m5.m2_0 = (_Bool)(abi_mix(s, 32u) & 1u);
    (*p).m5.m2_1 = (unsigned)(abi_mix(s, 33u) % 2048u);
    (*p).m5.m2_2 = (unsigned)(abi_mix(s, 34u) % 4194304u);
    (*p).m5.m2_3 = (_Bool)(abi_mix(s, 35u) & 1u);
    (*p).m5.m3.m0[0] = (float)((double)(long long)(abi_mix(s, 36u) % 4001u) / 8.0 - 250.0);
    (*p).m5.m3.m0[1] = (float)((double)(long long)(abi_mix(s, 37u) % 4001u) / 8.0 - 250.0);
    (*p).m5.m3.m0[2] = (float)((double)(long long)(abi_mix(s, 38u) % 4001u) / 8.0 - 250.0);
    (*p).m5.m3.m0[3] = (float)((double)(long long)(abi_mix(s, 39u) % 4001u) / 8.0 - 250.0);
    (*p).m5.m3.m1 = (unsigned short)(abi_mix(s, 40u) % 65536u);
    (*p).m5.m3.m2[0] = (long long)abi_mix(s, 41u);
    (*p).m5.m3.m2[1] = (long long)abi_mix(s, 42u);
    (*p).m5.m3.m3 = (short)((long long)(abi_mix(s, 43u) % 65536u) - 32768);
    (*p).m5.m4_0 = (unsigned)(abi_mix(s, 44u) % 4294967296u);
    (*p).m5.m4_1 = (unsigned)(abi_mix(s, 45u) % 1073741824u);
}
static int check_R10(const struct R10 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 1u) % 2147483648u);
    bad |= (*p).m1_1 != (int)((long long)(abi_mix(s, 2u) % 16384u) - 8192);
    bad |= (*p).m1_2 != (unsigned)(abi_mix(s, 3u) % 2097152u);
    bad |= (*p).m2_0 != (unsigned)(abi_mix(s, 4u) % 32u);
    bad |= (*p).m3.m0[0][0] != (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[0][1] != (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[1][0] != (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[1][1] != (double)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[2][0] != (double)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[2][1] != (double)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[3][0] != (double)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[3][1] != (double)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[4][0] != (double)((double)(long long)(abi_mix(s, 13u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0[4][1] != (double)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m1.m0[0] != (unsigned char)(abi_mix(s, 15u) % 256u);
    bad |= (*p).m3.m1.m0[1] != (unsigned char)(abi_mix(s, 16u) % 256u);
    bad |= (*p).m3.m1.m0[2] != (unsigned char)(abi_mix(s, 17u) % 256u);
    bad |= (*p).m3.m1.m0[3] != (unsigned char)(abi_mix(s, 18u) % 256u);
    bad |= (*p).m3.m1.m0[4] != (unsigned char)(abi_mix(s, 19u) % 256u);
    bad |= (*p).m3.m1.m0[5] != (unsigned char)(abi_mix(s, 20u) % 256u);
    bad |= (*p).m3.m1.m0[6] != (unsigned char)(abi_mix(s, 21u) % 256u);
    bad |= (*p).m3.m1.m0[7] != (unsigned char)(abi_mix(s, 22u) % 256u);
    bad |= (*p).m3.m2 != (unsigned short)(abi_mix(s, 23u) % 65536u);
    bad |= (*p).m3.m3_0 != (_Bool)(abi_mix(s, 24u) & 1u);
    bad |= (*p).m3.m4_0 != (_Bool)(abi_mix(s, 25u) & 1u);
    bad |= (*p).m3.m4_1 != (unsigned)(abi_mix(s, 26u) & 1u);
    bad |= (*p).m4 != (_Bool)(abi_mix(s, 27u) & 1u);
    bad |= (*p).m5.m0 != (abi_anchor + (abi_mix(s, 28u) % 8u));
    bad |= (*p).m5.m1[0] != (abi_anchor + (abi_mix(s, 29u) % 8u));
    bad |= (*p).m5.m1[1] != (abi_anchor + (abi_mix(s, 30u) % 8u));
    bad |= (*p).m5.m1[2] != (abi_anchor + (abi_mix(s, 31u) % 8u));
    bad |= (*p).m5.m2_0 != (_Bool)(abi_mix(s, 32u) & 1u);
    bad |= (*p).m5.m2_1 != (unsigned)(abi_mix(s, 33u) % 2048u);
    bad |= (*p).m5.m2_2 != (unsigned)(abi_mix(s, 34u) % 4194304u);
    bad |= (*p).m5.m2_3 != (_Bool)(abi_mix(s, 35u) & 1u);
    bad |= (*p).m5.m3.m0[0] != (float)((double)(long long)(abi_mix(s, 36u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5.m3.m0[1] != (float)((double)(long long)(abi_mix(s, 37u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5.m3.m0[2] != (float)((double)(long long)(abi_mix(s, 38u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5.m3.m0[3] != (float)((double)(long long)(abi_mix(s, 39u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5.m3.m1 != (unsigned short)(abi_mix(s, 40u) % 65536u);
    bad |= (*p).m5.m3.m2[0] != (long long)abi_mix(s, 41u);
    bad |= (*p).m5.m3.m2[1] != (long long)abi_mix(s, 42u);
    bad |= (*p).m5.m3.m3 != (short)((long long)(abi_mix(s, 43u) % 65536u) - 32768);
    bad |= (*p).m5.m4_0 != (unsigned)(abi_mix(s, 44u) % 4294967296u);
    bad |= (*p).m5.m4_1 != (unsigned)(abi_mix(s, 45u) % 1073741824u);
    return !bad;
}
static void fill_R11(struct R11 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R11(const struct R11 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R12(struct R12 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0][0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[0][1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0[0] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0[1] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1[0] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1[1] = (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m2[1] = (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m3[0] = (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1] = (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
}
static int check_R12(const struct R12 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0][0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[0][1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0[0] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0[1] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1[0] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1[1] != (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[1] != (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[0] != (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1] != (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R13(struct R13 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2[1] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3[0] = (long long)abi_mix(s, 4u);
    (*p).m3[1] = (long long)abi_mix(s, 5u);
    (*p).m4 = (signed char)((long long)(abi_mix(s, 6u) % 256u) - 128);
}
static int check_R13(const struct R13 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[1] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[0] != (long long)abi_mix(s, 4u);
    bad |= (*p).m3[1] != (long long)abi_mix(s, 5u);
    bad |= (*p).m4 != (signed char)((long long)(abi_mix(s, 6u) % 256u) - 128);
    return !bad;
}
static void fill_R14(struct R14 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1[2] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1[3] = (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R14(const struct R14 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[2] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[3] != (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R15(struct R15 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m2[1] = (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
}
static int check_R15(const struct R15 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[1] != (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R16(struct R16 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0][0] = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p).m2[0][1] = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p).m2[0][2] = (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    (*p).m2[0][3] = (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    (*p).m2[1][0] = (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
    (*p).m2[1][1] = (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    (*p).m2[1][2] = (int)((long long)(abi_mix(s, 9u) % 4294967296u) - 2147483648);
    (*p).m2[1][3] = (int)((long long)(abi_mix(s, 10u) % 4294967296u) - 2147483648);
    (*p).m2[2][0] = (int)((long long)(abi_mix(s, 11u) % 4294967296u) - 2147483648);
    (*p).m2[2][1] = (int)((long long)(abi_mix(s, 12u) % 4294967296u) - 2147483648);
    (*p).m2[2][2] = (int)((long long)(abi_mix(s, 13u) % 4294967296u) - 2147483648);
    (*p).m2[2][3] = (int)((long long)(abi_mix(s, 14u) % 4294967296u) - 2147483648);
    (*p).m2[3][0] = (int)((long long)(abi_mix(s, 15u) % 4294967296u) - 2147483648);
    (*p).m2[3][1] = (int)((long long)(abi_mix(s, 16u) % 4294967296u) - 2147483648);
    (*p).m2[3][2] = (int)((long long)(abi_mix(s, 17u) % 4294967296u) - 2147483648);
    (*p).m2[3][3] = (int)((long long)(abi_mix(s, 18u) % 4294967296u) - 2147483648);
    (*p).m2[4][0] = (int)((long long)(abi_mix(s, 19u) % 4294967296u) - 2147483648);
    (*p).m2[4][1] = (int)((long long)(abi_mix(s, 20u) % 4294967296u) - 2147483648);
    (*p).m2[4][2] = (int)((long long)(abi_mix(s, 21u) % 4294967296u) - 2147483648);
    (*p).m2[4][3] = (int)((long long)(abi_mix(s, 22u) % 4294967296u) - 2147483648);
    (*p).m3_0 = (int)((long long)(abi_mix(s, 23u) % 32u) - 16);
    (*p).m3_1 = (_Bool)(abi_mix(s, 24u) & 1u);
}
static int check_R16(const struct R16 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0][0] != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[0][1] != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[0][2] != (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[0][3] != (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[1][0] != (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[1][1] != (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[1][2] != (int)((long long)(abi_mix(s, 9u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[1][3] != (int)((long long)(abi_mix(s, 10u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[2][0] != (int)((long long)(abi_mix(s, 11u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[2][1] != (int)((long long)(abi_mix(s, 12u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[2][2] != (int)((long long)(abi_mix(s, 13u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[2][3] != (int)((long long)(abi_mix(s, 14u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[3][0] != (int)((long long)(abi_mix(s, 15u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[3][1] != (int)((long long)(abi_mix(s, 16u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[3][2] != (int)((long long)(abi_mix(s, 17u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[3][3] != (int)((long long)(abi_mix(s, 18u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[4][0] != (int)((long long)(abi_mix(s, 19u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[4][1] != (int)((long long)(abi_mix(s, 20u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[4][2] != (int)((long long)(abi_mix(s, 21u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[4][3] != (int)((long long)(abi_mix(s, 22u) % 4294967296u) - 2147483648);
    bad |= (*p).m3_0 != (int)((long long)(abi_mix(s, 23u) % 32u) - 16);
    bad |= (*p).m3_1 != (_Bool)(abi_mix(s, 24u) & 1u);
    return !bad;
}
static void fill_R17(union R17 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m0[1] = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m0[2] = (unsigned short)(abi_mix(s, 2u) % 65536u);
    (*p).m0[3] = (unsigned short)(abi_mix(s, 3u) % 65536u);
    (*p).m0[4] = (unsigned short)(abi_mix(s, 4u) % 65536u);
}
static int check_R17(const union R17 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m0[1] != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m0[2] != (unsigned short)(abi_mix(s, 2u) % 65536u);
    bad |= (*p).m0[3] != (unsigned short)(abi_mix(s, 3u) % 65536u);
    bad |= (*p).m0[4] != (unsigned short)(abi_mix(s, 4u) % 65536u);
    return !bad;
}
static void fill_R18(struct R18 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m1 = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    (*p).m2[0][0] = (long long)abi_mix(s, 2u);
    (*p).m2[0][1] = (long long)abi_mix(s, 3u);
    (*p).m2[1][0] = (long long)abi_mix(s, 4u);
    (*p).m2[1][1] = (long long)abi_mix(s, 5u);
}
static int check_R18(const struct R18 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m1 != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    bad |= (*p).m2[0][0] != (long long)abi_mix(s, 2u);
    bad |= (*p).m2[0][1] != (long long)abi_mix(s, 3u);
    bad |= (*p).m2[1][0] != (long long)abi_mix(s, 4u);
    bad |= (*p).m2[1][1] != (long long)abi_mix(s, 5u);
    return !bad;
}
static void fill_R19(struct R19 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 33554432u) - 16777216);
    (*p).m0_1 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 8u) - 4);
    (*p).m0_3 = (int)((long long)(abi_mix(s, 3u) % 32u) - 16);
}
static int check_R19(const struct R19 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 33554432u) - 16777216);
    bad |= (*p).m0_1 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 8u) - 4);
    bad |= (*p).m0_3 != (int)((long long)(abi_mix(s, 3u) % 32u) - 16);
    return !bad;
}
static void fill_R20(struct R20 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0_0 = (int)((long long)(abi_mix(s, 0u) % 33554432u) - 16777216);
    (*p).m0.m0_1 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m0.m0_2 = (int)((long long)(abi_mix(s, 2u) % 8u) - 4);
    (*p).m0.m0_3 = (int)((long long)(abi_mix(s, 3u) % 32u) - 16);
    (*p).m1_0 = (unsigned)(abi_mix(s, 4u) % 128u);
    (*p).m1_1 = (int)((long long)(abi_mix(s, 5u) % 131072u) - 65536);
    (*p).m2 = (long long)abi_mix(s, 6u);
}
static int check_R20(const struct R20 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0_0 != (int)((long long)(abi_mix(s, 0u) % 33554432u) - 16777216);
    bad |= (*p).m0.m0_1 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m0.m0_2 != (int)((long long)(abi_mix(s, 2u) % 8u) - 4);
    bad |= (*p).m0.m0_3 != (int)((long long)(abi_mix(s, 3u) % 32u) - 16);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 4u) % 128u);
    bad |= (*p).m1_1 != (int)((long long)(abi_mix(s, 5u) % 131072u) - 65536);
    bad |= (*p).m2 != (long long)abi_mix(s, 6u);
    return !bad;
}
static void fill_R21(struct R21 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
}
static int check_R21(const struct R21 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    return !bad;
}
static void fill_R22(struct R22 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2_0 = (_Bool)(abi_mix(s, 3u) & 1u);
    (*p).m2_1 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m3[0] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m3[1] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m3[2] = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m3[3] = (unsigned char)(abi_mix(s, 8u) % 256u);
    (*p).m3[4] = (unsigned char)(abi_mix(s, 9u) % 256u);
    (*p).m3[5] = (unsigned char)(abi_mix(s, 10u) % 256u);
    (*p).m3[6] = (unsigned char)(abi_mix(s, 11u) % 256u);
    (*p).m3[7] = (unsigned char)(abi_mix(s, 12u) % 256u);
}
static int check_R22(const struct R22 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2_0 != (_Bool)(abi_mix(s, 3u) & 1u);
    bad |= (*p).m2_1 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m3[0] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m3[1] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m3[2] != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m3[3] != (unsigned char)(abi_mix(s, 8u) % 256u);
    bad |= (*p).m3[4] != (unsigned char)(abi_mix(s, 9u) % 256u);
    bad |= (*p).m3[5] != (unsigned char)(abi_mix(s, 10u) % 256u);
    bad |= (*p).m3[6] != (unsigned char)(abi_mix(s, 11u) % 256u);
    bad |= (*p).m3[7] != (unsigned char)(abi_mix(s, 12u) % 256u);
    return !bad;
}
static void fill_R23(struct R23 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m1[0] = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m1[1] = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p).m1[2] = (unsigned char)(abi_mix(s, 3u) % 256u);
    (*p).m1[3] = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p).m1[4] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m1[5] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m1[6] = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m1[7] = (unsigned char)(abi_mix(s, 8u) % 256u);
}
static int check_R23(const struct R23 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m1[0] != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m1[1] != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m1[2] != (unsigned char)(abi_mix(s, 3u) % 256u);
    bad |= (*p).m1[3] != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p).m1[4] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m1[5] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m1[6] != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m1[7] != (unsigned char)(abi_mix(s, 8u) % 256u);
    return !bad;
}
static void fill_R24(struct R24 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0.m0_0 = (int)((long long)(abi_mix(s, 0u) % 33554432u) - 16777216);
    (*p).m0.m0.m0_1 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m0.m0.m0_2 = (int)((long long)(abi_mix(s, 2u) % 8u) - 4);
    (*p).m0.m0.m0_3 = (int)((long long)(abi_mix(s, 3u) % 32u) - 16);
    (*p).m0.m1_0 = (unsigned)(abi_mix(s, 4u) % 128u);
    (*p).m0.m1_1 = (int)((long long)(abi_mix(s, 5u) % 131072u) - 65536);
    (*p).m0.m2 = (long long)abi_mix(s, 6u);
    (*p).m1_0 = (_Bool)(abi_mix(s, 7u) & 1u);
    (*p).m1_1 = (unsigned)(abi_mix(s, 8u) % 4294967296u);
    (*p).m2.m0.m0 = (signed char)((long long)(abi_mix(s, 9u) % 256u) - 128);
    (*p).m2.m1[0] = (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m1[1] = (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m2_0 = (_Bool)(abi_mix(s, 12u) & 1u);
    (*p).m2.m2_1 = (_Bool)(abi_mix(s, 13u) & 1u);
    (*p).m2.m3[0] = (unsigned char)(abi_mix(s, 14u) % 256u);
    (*p).m2.m3[1] = (unsigned char)(abi_mix(s, 15u) % 256u);
    (*p).m2.m3[2] = (unsigned char)(abi_mix(s, 16u) % 256u);
    (*p).m2.m3[3] = (unsigned char)(abi_mix(s, 17u) % 256u);
    (*p).m2.m3[4] = (unsigned char)(abi_mix(s, 18u) % 256u);
    (*p).m2.m3[5] = (unsigned char)(abi_mix(s, 19u) % 256u);
    (*p).m2.m3[6] = (unsigned char)(abi_mix(s, 20u) % 256u);
    (*p).m2.m3[7] = (unsigned char)(abi_mix(s, 21u) % 256u);
    (*p).m3_0 = (int)((long long)(abi_mix(s, 22u) % 32768u) - 16384);
    (*p).m4.m0 = (unsigned short)(abi_mix(s, 23u) % 65536u);
    (*p).m4.m1[0] = (unsigned char)(abi_mix(s, 24u) % 256u);
    (*p).m4.m1[1] = (unsigned char)(abi_mix(s, 25u) % 256u);
    (*p).m4.m1[2] = (unsigned char)(abi_mix(s, 26u) % 256u);
    (*p).m4.m1[3] = (unsigned char)(abi_mix(s, 27u) % 256u);
    (*p).m4.m1[4] = (unsigned char)(abi_mix(s, 28u) % 256u);
    (*p).m4.m1[5] = (unsigned char)(abi_mix(s, 29u) % 256u);
    (*p).m4.m1[6] = (unsigned char)(abi_mix(s, 30u) % 256u);
    (*p).m4.m1[7] = (unsigned char)(abi_mix(s, 31u) % 256u);
    (*p).m5 = (abi_anchor + (abi_mix(s, 32u) % 8u));
    (*p).m6[0] = (unsigned char)(abi_mix(s, 33u) % 256u);
    (*p).m6[1] = (unsigned char)(abi_mix(s, 34u) % 256u);
    (*p).m6[2] = (unsigned char)(abi_mix(s, 35u) % 256u);
    (*p).m6[3] = (unsigned char)(abi_mix(s, 36u) % 256u);
    (*p).m6[4] = (unsigned char)(abi_mix(s, 37u) % 256u);
    (*p).m6[5] = (unsigned char)(abi_mix(s, 38u) % 256u);
    (*p).m6[6] = (unsigned char)(abi_mix(s, 39u) % 256u);
    (*p).m6[7] = (unsigned char)(abi_mix(s, 40u) % 256u);
    (*p).m7 = (unsigned short)(abi_mix(s, 41u) % 65536u);
}
static int check_R24(const struct R24 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0.m0_0 != (int)((long long)(abi_mix(s, 0u) % 33554432u) - 16777216);
    bad |= (*p).m0.m0.m0_1 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m0.m0.m0_2 != (int)((long long)(abi_mix(s, 2u) % 8u) - 4);
    bad |= (*p).m0.m0.m0_3 != (int)((long long)(abi_mix(s, 3u) % 32u) - 16);
    bad |= (*p).m0.m1_0 != (unsigned)(abi_mix(s, 4u) % 128u);
    bad |= (*p).m0.m1_1 != (int)((long long)(abi_mix(s, 5u) % 131072u) - 65536);
    bad |= (*p).m0.m2 != (long long)abi_mix(s, 6u);
    bad |= (*p).m1_0 != (_Bool)(abi_mix(s, 7u) & 1u);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 8u) % 4294967296u);
    bad |= (*p).m2.m0.m0 != (signed char)((long long)(abi_mix(s, 9u) % 256u) - 128);
    bad |= (*p).m2.m1[0] != (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m1[1] != (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m2_0 != (_Bool)(abi_mix(s, 12u) & 1u);
    bad |= (*p).m2.m2_1 != (_Bool)(abi_mix(s, 13u) & 1u);
    bad |= (*p).m2.m3[0] != (unsigned char)(abi_mix(s, 14u) % 256u);
    bad |= (*p).m2.m3[1] != (unsigned char)(abi_mix(s, 15u) % 256u);
    bad |= (*p).m2.m3[2] != (unsigned char)(abi_mix(s, 16u) % 256u);
    bad |= (*p).m2.m3[3] != (unsigned char)(abi_mix(s, 17u) % 256u);
    bad |= (*p).m2.m3[4] != (unsigned char)(abi_mix(s, 18u) % 256u);
    bad |= (*p).m2.m3[5] != (unsigned char)(abi_mix(s, 19u) % 256u);
    bad |= (*p).m2.m3[6] != (unsigned char)(abi_mix(s, 20u) % 256u);
    bad |= (*p).m2.m3[7] != (unsigned char)(abi_mix(s, 21u) % 256u);
    bad |= (*p).m3_0 != (int)((long long)(abi_mix(s, 22u) % 32768u) - 16384);
    bad |= (*p).m4.m0 != (unsigned short)(abi_mix(s, 23u) % 65536u);
    bad |= (*p).m4.m1[0] != (unsigned char)(abi_mix(s, 24u) % 256u);
    bad |= (*p).m4.m1[1] != (unsigned char)(abi_mix(s, 25u) % 256u);
    bad |= (*p).m4.m1[2] != (unsigned char)(abi_mix(s, 26u) % 256u);
    bad |= (*p).m4.m1[3] != (unsigned char)(abi_mix(s, 27u) % 256u);
    bad |= (*p).m4.m1[4] != (unsigned char)(abi_mix(s, 28u) % 256u);
    bad |= (*p).m4.m1[5] != (unsigned char)(abi_mix(s, 29u) % 256u);
    bad |= (*p).m4.m1[6] != (unsigned char)(abi_mix(s, 30u) % 256u);
    bad |= (*p).m4.m1[7] != (unsigned char)(abi_mix(s, 31u) % 256u);
    bad |= (*p).m5 != (abi_anchor + (abi_mix(s, 32u) % 8u));
    bad |= (*p).m6[0] != (unsigned char)(abi_mix(s, 33u) % 256u);
    bad |= (*p).m6[1] != (unsigned char)(abi_mix(s, 34u) % 256u);
    bad |= (*p).m6[2] != (unsigned char)(abi_mix(s, 35u) % 256u);
    bad |= (*p).m6[3] != (unsigned char)(abi_mix(s, 36u) % 256u);
    bad |= (*p).m6[4] != (unsigned char)(abi_mix(s, 37u) % 256u);
    bad |= (*p).m6[5] != (unsigned char)(abi_mix(s, 38u) % 256u);
    bad |= (*p).m6[6] != (unsigned char)(abi_mix(s, 39u) % 256u);
    bad |= (*p).m6[7] != (unsigned char)(abi_mix(s, 40u) % 256u);
    bad |= (*p).m7 != (unsigned short)(abi_mix(s, 41u) % 65536u);
    return !bad;
}
static void fill_R25(struct R25 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (unsigned long long)abi_mix(s, 2u);
    (*p).m2 = (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
}
static int check_R25(const struct R25 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (unsigned long long)abi_mix(s, 2u);
    bad |= (*p).m2 != (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
    return !bad;
}
static void fill_R26(struct R26 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (_Float16)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R26(const struct R26 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (_Float16)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R27(struct R27 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (_Float16)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R27(const struct R27 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (_Float16)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R28(struct R28 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
}
static int check_R28(const struct R28 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    return !bad;
}
static void fill_R29(struct R29 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R29(const struct R29 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R30(union R30 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R30(const union R30 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R31(struct R31 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0.m0[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0.m0[1] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0.m1 = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R31(const struct R31 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0.m0[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0.m0[1] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0.m1 != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R32(union R32 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
}
static int check_R32(const union R32 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    return !bad;
}
static void fill_R33(struct R33 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 1024u) - 512);
    (*p).m0_1 = (int)((long long)(abi_mix(s, 1u) % 134217728u) - 67108864);
    (*p).m1[0] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (unsigned short)(abi_mix(s, 4u) % 65536u);
}
static int check_R33(const struct R33 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 1024u) - 512);
    bad |= (*p).m0_1 != (int)((long long)(abi_mix(s, 1u) % 134217728u) - 67108864);
    bad |= (*p).m1[0] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (unsigned short)(abi_mix(s, 4u) % 65536u);
    return !bad;
}
static void fill_R34(struct R34 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0_0 = (int)((long long)(abi_mix(s, 0u) % 1024u) - 512);
    (*p).m0.m0_1 = (int)((long long)(abi_mix(s, 1u) % 134217728u) - 67108864);
    (*p).m0.m1[0] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1[1] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m2 = (unsigned short)(abi_mix(s, 4u) % 65536u);
}
static int check_R34(const struct R34 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0_0 != (int)((long long)(abi_mix(s, 0u) % 1024u) - 512);
    bad |= (*p).m0.m0_1 != (int)((long long)(abi_mix(s, 1u) % 134217728u) - 67108864);
    bad |= (*p).m0.m1[0] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1[1] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m2 != (unsigned short)(abi_mix(s, 4u) % 65536u);
    return !bad;
}
static void fill_R35(struct R35 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
}
static int check_R35(const struct R35 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    return !bad;
}
static void fill_R36(struct R36 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (long long)abi_mix(s, 2u);
    (*p).m1[1] = (long long)abi_mix(s, 3u);
    (*p).m1[2] = (long long)abi_mix(s, 4u);
    (*p).m1[3] = (long long)abi_mix(s, 5u);
    (*p).m1[4] = (long long)abi_mix(s, 6u);
    (*p).m2[0] = (signed char)((long long)(abi_mix(s, 7u) % 256u) - 128);
    (*p).m2[1] = (signed char)((long long)(abi_mix(s, 8u) % 256u) - 128);
    (*p).m3 = (_Float16)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    (*p).m4.m0 = (unsigned short)(abi_mix(s, 10u) % 65536u);
}
static int check_R36(const struct R36 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (long long)abi_mix(s, 2u);
    bad |= (*p).m1[1] != (long long)abi_mix(s, 3u);
    bad |= (*p).m1[2] != (long long)abi_mix(s, 4u);
    bad |= (*p).m1[3] != (long long)abi_mix(s, 5u);
    bad |= (*p).m1[4] != (long long)abi_mix(s, 6u);
    bad |= (*p).m2[0] != (signed char)((long long)(abi_mix(s, 7u) % 256u) - 128);
    bad |= (*p).m2[1] != (signed char)((long long)(abi_mix(s, 8u) % 256u) - 128);
    bad |= (*p).m3 != (_Float16)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4.m0 != (unsigned short)(abi_mix(s, 10u) % 65536u);
    return !bad;
}
static void fill_R37(struct R37 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R37(const struct R37 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R38(struct R38 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R38(const struct R38 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R39(struct R39 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R39(const struct R39 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
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
static void fill_R41(struct R41 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 262144u);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 33554432u);
    (*p).m0_2 = (unsigned)(abi_mix(s, 2u) % 2147483648u);
    (*p).m0_3 = (_Bool)(abi_mix(s, 3u) & 1u);
    (*p).m1 = (unsigned short)(abi_mix(s, 4u) % 65536u);
}
static int check_R41(const struct R41 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 262144u);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 33554432u);
    bad |= (*p).m0_2 != (unsigned)(abi_mix(s, 2u) % 2147483648u);
    bad |= (*p).m0_3 != (_Bool)(abi_mix(s, 3u) & 1u);
    bad |= (*p).m1 != (unsigned short)(abi_mix(s, 4u) % 65536u);
    return !bad;
}
static void fill_R42(struct R42 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (long long)abi_mix(s, 1u);
    (*p).m2[0] = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m2[1] = (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    (*p).m2[2] = (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
    (*p).m2[3] = (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    (*p).m3 = (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m4.m0_0 = (unsigned)(abi_mix(s, 7u) % 262144u);
    (*p).m4.m0_1 = (unsigned)(abi_mix(s, 8u) % 33554432u);
    (*p).m4.m0_2 = (unsigned)(abi_mix(s, 9u) % 2147483648u);
    (*p).m4.m0_3 = (_Bool)(abi_mix(s, 10u) & 1u);
    (*p).m4.m1 = (unsigned short)(abi_mix(s, 11u) % 65536u);
    (*p).m5[0] = (signed char)((long long)(abi_mix(s, 12u) % 256u) - 128);
    (*p).m5[1] = (signed char)((long long)(abi_mix(s, 13u) % 256u) - 128);
    (*p).m5[2] = (signed char)((long long)(abi_mix(s, 14u) % 256u) - 128);
    (*p).m5[3] = (signed char)((long long)(abi_mix(s, 15u) % 256u) - 128);
}
static int check_R42(const struct R42 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (long long)abi_mix(s, 1u);
    bad |= (*p).m2[0] != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m2[1] != (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    bad |= (*p).m2[2] != (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
    bad |= (*p).m2[3] != (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    bad |= (*p).m3 != (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4.m0_0 != (unsigned)(abi_mix(s, 7u) % 262144u);
    bad |= (*p).m4.m0_1 != (unsigned)(abi_mix(s, 8u) % 33554432u);
    bad |= (*p).m4.m0_2 != (unsigned)(abi_mix(s, 9u) % 2147483648u);
    bad |= (*p).m4.m0_3 != (_Bool)(abi_mix(s, 10u) & 1u);
    bad |= (*p).m4.m1 != (unsigned short)(abi_mix(s, 11u) % 65536u);
    bad |= (*p).m5[0] != (signed char)((long long)(abi_mix(s, 12u) % 256u) - 128);
    bad |= (*p).m5[1] != (signed char)((long long)(abi_mix(s, 13u) % 256u) - 128);
    bad |= (*p).m5[2] != (signed char)((long long)(abi_mix(s, 14u) % 256u) - 128);
    bad |= (*p).m5[3] != (signed char)((long long)(abi_mix(s, 15u) % 256u) - 128);
    return !bad;
}
static void fill_R43(struct R43 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R43(const struct R43 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R44(struct R44 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m4 = (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R44(const struct R44 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4 != (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R45(union R45 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
}
static int check_R45(const union R45 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    return !bad;
}
static void fill_R46(struct R46 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
}
static int check_R46(const struct R46 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    return !bad;
}
static void fill_R47(struct R47 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0.m0 = (unsigned char)(abi_mix(s, 1u) % 256u);
}
static int check_R47(const struct R47 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0.m0 != (unsigned char)(abi_mix(s, 1u) % 256u);
    return !bad;
}
static void fill_R48(struct R48 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 16384u) - 8192);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 4096u);
    (*p).m0_2 = (unsigned)(abi_mix(s, 2u) % 268435456u);
    (*p).m0_3 = (int)((long long)(abi_mix(s, 3u) % 4096u) - 2048);
}
static int check_R48(const struct R48 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 16384u) - 8192);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 4096u);
    bad |= (*p).m0_2 != (unsigned)(abi_mix(s, 2u) % 268435456u);
    bad |= (*p).m0_3 != (int)((long long)(abi_mix(s, 3u) % 4096u) - 2048);
    return !bad;
}
static void fill_R49(struct R49 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3_0 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m3_1 = (int)((long long)(abi_mix(s, 5u) % 4194304u) - 2097152);
    (*p).m3_2 = (int)((long long)(abi_mix(s, 6u) % 524288u) - 262144);
    (*p).m3_3 = (unsigned)(abi_mix(s, 7u) % 65536u);
    (*p).m4 = (long long)abi_mix(s, 8u);
}
static int check_R49(const struct R49 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3_0 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m3_1 != (int)((long long)(abi_mix(s, 5u) % 4194304u) - 2097152);
    bad |= (*p).m3_2 != (int)((long long)(abi_mix(s, 6u) % 524288u) - 262144);
    bad |= (*p).m3_3 != (unsigned)(abi_mix(s, 7u) % 65536u);
    bad |= (*p).m4 != (long long)abi_mix(s, 8u);
    return !bad;
}
static void fill_R50(struct R50 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (long long)abi_mix(s, 1u);
}
static int check_R50(const struct R50 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (long long)abi_mix(s, 1u);
    return !bad;
}
static void fill_R51(struct R51 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (unsigned)(abi_mix(s, 2u) % 4294967296u);
    (*p).m3[0] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R51(const struct R51 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (unsigned)(abi_mix(s, 2u) % 4294967296u);
    bad |= (*p).m3[0] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R52(union R52 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
}
static int check_R52(const union R52 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R53(struct R53 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m0_1 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 134217728u) - 67108864);
    (*p).m1_0 = (int)((long long)(abi_mix(s, 3u) % 536870912u) - 268435456);
    (*p).m2[0] = (long long)abi_mix(s, 4u);
    (*p).m2[1] = (long long)abi_mix(s, 5u);
    (*p).m3_0 = (int)((long long)(abi_mix(s, 6u) % 32u) - 16);
    (*p).m4_0 = (int)((long long)(abi_mix(s, 7u) % 33554432u) - 16777216);
    (*p).m4_1 = (unsigned)(abi_mix(s, 8u) % 512u);
    (*p).m4_2 = (unsigned)(abi_mix(s, 9u) % 16384u);
    (*p).m4_3 = (unsigned)(abi_mix(s, 10u) % 1048576u);
    (*p).m5[0] = (unsigned long long)abi_mix(s, 11u);
    (*p).m6 = (signed char)((long long)(abi_mix(s, 12u) % 256u) - 128);
    (*p).m7.m0 = (int)((long long)(abi_mix(s, 13u) % 4294967296u) - 2147483648);
}
static int check_R53(const struct R53 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m0_1 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 134217728u) - 67108864);
    bad |= (*p).m1_0 != (int)((long long)(abi_mix(s, 3u) % 536870912u) - 268435456);
    bad |= (*p).m2[0] != (long long)abi_mix(s, 4u);
    bad |= (*p).m2[1] != (long long)abi_mix(s, 5u);
    bad |= (*p).m3_0 != (int)((long long)(abi_mix(s, 6u) % 32u) - 16);
    bad |= (*p).m4_0 != (int)((long long)(abi_mix(s, 7u) % 33554432u) - 16777216);
    bad |= (*p).m4_1 != (unsigned)(abi_mix(s, 8u) % 512u);
    bad |= (*p).m4_2 != (unsigned)(abi_mix(s, 9u) % 16384u);
    bad |= (*p).m4_3 != (unsigned)(abi_mix(s, 10u) % 1048576u);
    bad |= (*p).m5[0] != (unsigned long long)abi_mix(s, 11u);
    bad |= (*p).m6 != (signed char)((long long)(abi_mix(s, 12u) % 256u) - 128);
    bad |= (*p).m7.m0 != (int)((long long)(abi_mix(s, 13u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R54(struct R54 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p).m0[1] = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m0[2] = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p).m0[3] = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
}
static int check_R54(const struct R54 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[1] != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[2] != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[3] != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R55(struct R55 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m1[1] = (_Bool)(abi_mix(s, 3u) & 1u);
    (*p).m1[2] = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m1[3] = (_Bool)(abi_mix(s, 5u) & 1u);
}
static int check_R55(const struct R55 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m1[1] != (_Bool)(abi_mix(s, 3u) & 1u);
    bad |= (*p).m1[2] != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m1[3] != (_Bool)(abi_mix(s, 5u) & 1u);
    return !bad;
}
static void fill_R56(struct R56 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0[1] = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m0[2] = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m0[3] = (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
}
static int check_R56(const struct R56 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0[1] != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m0[2] != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m0[3] != (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    return !bad;
}
static void fill_R57(struct R57 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m2[1] = (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R57(const struct R57 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[1] != (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R58(union R58 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (long long)abi_mix(s, 0u);
    (*p).m0[1] = (long long)abi_mix(s, 1u);
}
static int check_R58(const union R58 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (long long)abi_mix(s, 0u);
    bad |= (*p).m0[1] != (long long)abi_mix(s, 1u);
    return !bad;
}
static void fill_R59(struct R59 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m1.m0 = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m1.m1 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2 = (unsigned)(abi_mix(s, 3u) % 4294967296u);
    (*p).m1.m3[0] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m3[1] = (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0].m0_0 = (_Bool)(abi_mix(s, 6u) & 1u);
    (*p).m2[0].m0_1 = (_Bool)(abi_mix(s, 7u) & 1u);
    (*p).m2[0].m0_2 = (int)((long long)(abi_mix(s, 8u) % 134217728u) - 67108864);
    (*p).m2[0].m1_0 = (int)((long long)(abi_mix(s, 9u) % 536870912u) - 268435456);
    (*p).m2[0].m2[0] = (long long)abi_mix(s, 10u);
    (*p).m2[0].m2[1] = (long long)abi_mix(s, 11u);
    (*p).m2[0].m3_0 = (int)((long long)(abi_mix(s, 12u) % 32u) - 16);
    (*p).m2[0].m4_0 = (int)((long long)(abi_mix(s, 13u) % 33554432u) - 16777216);
    (*p).m2[0].m4_1 = (unsigned)(abi_mix(s, 14u) % 512u);
    (*p).m2[0].m4_2 = (unsigned)(abi_mix(s, 15u) % 16384u);
    (*p).m2[0].m4_3 = (unsigned)(abi_mix(s, 16u) % 1048576u);
    (*p).m2[0].m5[0] = (unsigned long long)abi_mix(s, 17u);
    (*p).m2[0].m6 = (signed char)((long long)(abi_mix(s, 18u) % 256u) - 128);
    (*p).m2[0].m7.m0 = (int)((long long)(abi_mix(s, 19u) % 4294967296u) - 2147483648);
    (*p).m2[1].m0_0 = (_Bool)(abi_mix(s, 20u) & 1u);
    (*p).m2[1].m0_1 = (_Bool)(abi_mix(s, 21u) & 1u);
    (*p).m2[1].m0_2 = (int)((long long)(abi_mix(s, 22u) % 134217728u) - 67108864);
    (*p).m2[1].m1_0 = (int)((long long)(abi_mix(s, 23u) % 536870912u) - 268435456);
    (*p).m2[1].m2[0] = (long long)abi_mix(s, 24u);
    (*p).m2[1].m2[1] = (long long)abi_mix(s, 25u);
    (*p).m2[1].m3_0 = (int)((long long)(abi_mix(s, 26u) % 32u) - 16);
    (*p).m2[1].m4_0 = (int)((long long)(abi_mix(s, 27u) % 33554432u) - 16777216);
    (*p).m2[1].m4_1 = (unsigned)(abi_mix(s, 28u) % 512u);
    (*p).m2[1].m4_2 = (unsigned)(abi_mix(s, 29u) % 16384u);
    (*p).m2[1].m4_3 = (unsigned)(abi_mix(s, 30u) % 1048576u);
    (*p).m2[1].m5[0] = (unsigned long long)abi_mix(s, 31u);
    (*p).m2[1].m6 = (signed char)((long long)(abi_mix(s, 32u) % 256u) - 128);
    (*p).m2[1].m7.m0 = (int)((long long)(abi_mix(s, 33u) % 4294967296u) - 2147483648);
    (*p).m2[2].m0_0 = (_Bool)(abi_mix(s, 34u) & 1u);
    (*p).m2[2].m0_1 = (_Bool)(abi_mix(s, 35u) & 1u);
    (*p).m2[2].m0_2 = (int)((long long)(abi_mix(s, 36u) % 134217728u) - 67108864);
    (*p).m2[2].m1_0 = (int)((long long)(abi_mix(s, 37u) % 536870912u) - 268435456);
    (*p).m2[2].m2[0] = (long long)abi_mix(s, 38u);
    (*p).m2[2].m2[1] = (long long)abi_mix(s, 39u);
    (*p).m2[2].m3_0 = (int)((long long)(abi_mix(s, 40u) % 32u) - 16);
    (*p).m2[2].m4_0 = (int)((long long)(abi_mix(s, 41u) % 33554432u) - 16777216);
    (*p).m2[2].m4_1 = (unsigned)(abi_mix(s, 42u) % 512u);
    (*p).m2[2].m4_2 = (unsigned)(abi_mix(s, 43u) % 16384u);
    (*p).m2[2].m4_3 = (unsigned)(abi_mix(s, 44u) % 1048576u);
    (*p).m2[2].m5[0] = (unsigned long long)abi_mix(s, 45u);
    (*p).m2[2].m6 = (signed char)((long long)(abi_mix(s, 46u) % 256u) - 128);
    (*p).m2[2].m7.m0 = (int)((long long)(abi_mix(s, 47u) % 4294967296u) - 2147483648);
    (*p).m3.m0[0] = (int)((long long)(abi_mix(s, 48u) % 4294967296u) - 2147483648);
    (*p).m3.m0[1] = (int)((long long)(abi_mix(s, 49u) % 4294967296u) - 2147483648);
    (*p).m3.m0[2] = (int)((long long)(abi_mix(s, 50u) % 4294967296u) - 2147483648);
    (*p).m3.m0[3] = (int)((long long)(abi_mix(s, 51u) % 4294967296u) - 2147483648);
    (*p).m4.m0[0] = (double)((double)(long long)(abi_mix(s, 52u) % 4001u) / 8.0 - 250.0);
    (*p).m4.m0[1] = (double)((double)(long long)(abi_mix(s, 53u) % 4001u) / 8.0 - 250.0);
    (*p).m4.m1[0] = (_Bool)(abi_mix(s, 54u) & 1u);
    (*p).m4.m1[1] = (_Bool)(abi_mix(s, 55u) & 1u);
    (*p).m4.m1[2] = (_Bool)(abi_mix(s, 56u) & 1u);
    (*p).m4.m1[3] = (_Bool)(abi_mix(s, 57u) & 1u);
    (*p).m5.m0[0] = (signed char)((long long)(abi_mix(s, 58u) % 256u) - 128);
    (*p).m5.m0[1] = (signed char)((long long)(abi_mix(s, 59u) % 256u) - 128);
    (*p).m5.m0[2] = (signed char)((long long)(abi_mix(s, 60u) % 256u) - 128);
    (*p).m5.m0[3] = (signed char)((long long)(abi_mix(s, 61u) % 256u) - 128);
    (*p).m6.m0[0] = (long long)abi_mix(s, 62u);
    (*p).m6.m0[1] = (long long)abi_mix(s, 63u);
    (*p).m7_0 = (_Bool)(abi_mix(s, 64u) & 1u);
    (*p).m7_1 = (int)((long long)(abi_mix(s, 65u) % 2u) - 1);
    (*p).m7_2 = (_Bool)(abi_mix(s, 66u) & 1u);
}
static int check_R59(const struct R59 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m1.m0 != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m1.m1 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2 != (unsigned)(abi_mix(s, 3u) % 4294967296u);
    bad |= (*p).m1.m3[0] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m3[1] != (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0].m0_0 != (_Bool)(abi_mix(s, 6u) & 1u);
    bad |= (*p).m2[0].m0_1 != (_Bool)(abi_mix(s, 7u) & 1u);
    bad |= (*p).m2[0].m0_2 != (int)((long long)(abi_mix(s, 8u) % 134217728u) - 67108864);
    bad |= (*p).m2[0].m1_0 != (int)((long long)(abi_mix(s, 9u) % 536870912u) - 268435456);
    bad |= (*p).m2[0].m2[0] != (long long)abi_mix(s, 10u);
    bad |= (*p).m2[0].m2[1] != (long long)abi_mix(s, 11u);
    bad |= (*p).m2[0].m3_0 != (int)((long long)(abi_mix(s, 12u) % 32u) - 16);
    bad |= (*p).m2[0].m4_0 != (int)((long long)(abi_mix(s, 13u) % 33554432u) - 16777216);
    bad |= (*p).m2[0].m4_1 != (unsigned)(abi_mix(s, 14u) % 512u);
    bad |= (*p).m2[0].m4_2 != (unsigned)(abi_mix(s, 15u) % 16384u);
    bad |= (*p).m2[0].m4_3 != (unsigned)(abi_mix(s, 16u) % 1048576u);
    bad |= (*p).m2[0].m5[0] != (unsigned long long)abi_mix(s, 17u);
    bad |= (*p).m2[0].m6 != (signed char)((long long)(abi_mix(s, 18u) % 256u) - 128);
    bad |= (*p).m2[0].m7.m0 != (int)((long long)(abi_mix(s, 19u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[1].m0_0 != (_Bool)(abi_mix(s, 20u) & 1u);
    bad |= (*p).m2[1].m0_1 != (_Bool)(abi_mix(s, 21u) & 1u);
    bad |= (*p).m2[1].m0_2 != (int)((long long)(abi_mix(s, 22u) % 134217728u) - 67108864);
    bad |= (*p).m2[1].m1_0 != (int)((long long)(abi_mix(s, 23u) % 536870912u) - 268435456);
    bad |= (*p).m2[1].m2[0] != (long long)abi_mix(s, 24u);
    bad |= (*p).m2[1].m2[1] != (long long)abi_mix(s, 25u);
    bad |= (*p).m2[1].m3_0 != (int)((long long)(abi_mix(s, 26u) % 32u) - 16);
    bad |= (*p).m2[1].m4_0 != (int)((long long)(abi_mix(s, 27u) % 33554432u) - 16777216);
    bad |= (*p).m2[1].m4_1 != (unsigned)(abi_mix(s, 28u) % 512u);
    bad |= (*p).m2[1].m4_2 != (unsigned)(abi_mix(s, 29u) % 16384u);
    bad |= (*p).m2[1].m4_3 != (unsigned)(abi_mix(s, 30u) % 1048576u);
    bad |= (*p).m2[1].m5[0] != (unsigned long long)abi_mix(s, 31u);
    bad |= (*p).m2[1].m6 != (signed char)((long long)(abi_mix(s, 32u) % 256u) - 128);
    bad |= (*p).m2[1].m7.m0 != (int)((long long)(abi_mix(s, 33u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[2].m0_0 != (_Bool)(abi_mix(s, 34u) & 1u);
    bad |= (*p).m2[2].m0_1 != (_Bool)(abi_mix(s, 35u) & 1u);
    bad |= (*p).m2[2].m0_2 != (int)((long long)(abi_mix(s, 36u) % 134217728u) - 67108864);
    bad |= (*p).m2[2].m1_0 != (int)((long long)(abi_mix(s, 37u) % 536870912u) - 268435456);
    bad |= (*p).m2[2].m2[0] != (long long)abi_mix(s, 38u);
    bad |= (*p).m2[2].m2[1] != (long long)abi_mix(s, 39u);
    bad |= (*p).m2[2].m3_0 != (int)((long long)(abi_mix(s, 40u) % 32u) - 16);
    bad |= (*p).m2[2].m4_0 != (int)((long long)(abi_mix(s, 41u) % 33554432u) - 16777216);
    bad |= (*p).m2[2].m4_1 != (unsigned)(abi_mix(s, 42u) % 512u);
    bad |= (*p).m2[2].m4_2 != (unsigned)(abi_mix(s, 43u) % 16384u);
    bad |= (*p).m2[2].m4_3 != (unsigned)(abi_mix(s, 44u) % 1048576u);
    bad |= (*p).m2[2].m5[0] != (unsigned long long)abi_mix(s, 45u);
    bad |= (*p).m2[2].m6 != (signed char)((long long)(abi_mix(s, 46u) % 256u) - 128);
    bad |= (*p).m2[2].m7.m0 != (int)((long long)(abi_mix(s, 47u) % 4294967296u) - 2147483648);
    bad |= (*p).m3.m0[0] != (int)((long long)(abi_mix(s, 48u) % 4294967296u) - 2147483648);
    bad |= (*p).m3.m0[1] != (int)((long long)(abi_mix(s, 49u) % 4294967296u) - 2147483648);
    bad |= (*p).m3.m0[2] != (int)((long long)(abi_mix(s, 50u) % 4294967296u) - 2147483648);
    bad |= (*p).m3.m0[3] != (int)((long long)(abi_mix(s, 51u) % 4294967296u) - 2147483648);
    bad |= (*p).m4.m0[0] != (double)((double)(long long)(abi_mix(s, 52u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4.m0[1] != (double)((double)(long long)(abi_mix(s, 53u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4.m1[0] != (_Bool)(abi_mix(s, 54u) & 1u);
    bad |= (*p).m4.m1[1] != (_Bool)(abi_mix(s, 55u) & 1u);
    bad |= (*p).m4.m1[2] != (_Bool)(abi_mix(s, 56u) & 1u);
    bad |= (*p).m4.m1[3] != (_Bool)(abi_mix(s, 57u) & 1u);
    bad |= (*p).m5.m0[0] != (signed char)((long long)(abi_mix(s, 58u) % 256u) - 128);
    bad |= (*p).m5.m0[1] != (signed char)((long long)(abi_mix(s, 59u) % 256u) - 128);
    bad |= (*p).m5.m0[2] != (signed char)((long long)(abi_mix(s, 60u) % 256u) - 128);
    bad |= (*p).m5.m0[3] != (signed char)((long long)(abi_mix(s, 61u) % 256u) - 128);
    bad |= (*p).m6.m0[0] != (long long)abi_mix(s, 62u);
    bad |= (*p).m6.m0[1] != (long long)abi_mix(s, 63u);
    bad |= (*p).m7_0 != (_Bool)(abi_mix(s, 64u) & 1u);
    bad |= (*p).m7_1 != (int)((long long)(abi_mix(s, 65u) % 2u) - 1);
    bad |= (*p).m7_2 != (_Bool)(abi_mix(s, 66u) & 1u);
    return !bad;
}
static void fill_R60(struct R60 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R60(const struct R60 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R61(struct R61 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0][0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[0][1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1][0] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1][1] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2][0] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2][1] = (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3][0] = (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3][1] = (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
}
static int check_R61(const struct R61 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0][0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[0][1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1][0] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1][1] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2][0] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2][1] != (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3][0] != (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3][1] != (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R62(union R62 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
}
static int check_R62(const union R62 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R63(struct R63 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m1 = (unsigned char)(abi_mix(s, 1u) % 256u);
}
static int check_R63(const struct R63 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m1 != (unsigned char)(abi_mix(s, 1u) % 256u);
    return !bad;
}
static void fill_R64(union R64 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
}
static int check_R64(const union R64 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    return !bad;
}
static void fill_R65(union R65 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
}
static int check_R65(const union R65 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    return !bad;
}
static void fill_R66(struct R66 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0[1] = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m1.m0 = (unsigned short)(abi_mix(s, 2u) % 65536u);
    (*p).m2 = (unsigned short)(abi_mix(s, 3u) % 65536u);
}
static int check_R66(const struct R66 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0[1] != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m1.m0 != (unsigned short)(abi_mix(s, 2u) % 65536u);
    bad |= (*p).m2 != (unsigned short)(abi_mix(s, 3u) % 65536u);
    return !bad;
}
static void fill_R67(struct R67 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
    (*p).m1 = (long long)abi_mix(s, 1u);
    (*p).m2[0][0] = (long long)abi_mix(s, 2u);
    (*p).m2[0][1] = (long long)abi_mix(s, 3u);
    (*p).m2[1][0] = (long long)abi_mix(s, 4u);
    (*p).m2[1][1] = (long long)abi_mix(s, 5u);
    (*p).m2[2][0] = (long long)abi_mix(s, 6u);
    (*p).m2[2][1] = (long long)abi_mix(s, 7u);
    (*p).m2[3][0] = (long long)abi_mix(s, 8u);
    (*p).m2[3][1] = (long long)abi_mix(s, 9u);
}
static int check_R67(const struct R67 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    bad |= (*p).m1 != (long long)abi_mix(s, 1u);
    bad |= (*p).m2[0][0] != (long long)abi_mix(s, 2u);
    bad |= (*p).m2[0][1] != (long long)abi_mix(s, 3u);
    bad |= (*p).m2[1][0] != (long long)abi_mix(s, 4u);
    bad |= (*p).m2[1][1] != (long long)abi_mix(s, 5u);
    bad |= (*p).m2[2][0] != (long long)abi_mix(s, 6u);
    bad |= (*p).m2[2][1] != (long long)abi_mix(s, 7u);
    bad |= (*p).m2[3][0] != (long long)abi_mix(s, 8u);
    bad |= (*p).m2[3][1] != (long long)abi_mix(s, 9u);
    return !bad;
}
static void fill_R68(struct R68 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m0[1] = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m0[2] = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p).m0[3] = (unsigned char)(abi_mix(s, 3u) % 256u);
    (*p).m0[4] = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p).m0[5] = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m0[6] = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m0[7] = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m1 = (unsigned char)(abi_mix(s, 8u) % 256u);
    (*p).m2 = (abi_anchor + (abi_mix(s, 9u) % 8u));
    (*p).m3[0] = (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1] = (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
}
static int check_R68(const struct R68 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m0[1] != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m0[2] != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m0[3] != (unsigned char)(abi_mix(s, 3u) % 256u);
    bad |= (*p).m0[4] != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p).m0[5] != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m0[6] != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m0[7] != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m1 != (unsigned char)(abi_mix(s, 8u) % 256u);
    bad |= (*p).m2 != (abi_anchor + (abi_mix(s, 9u) % 8u));
    bad |= (*p).m3[0] != (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1] != (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R69(struct R69 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (long long)abi_mix(s, 0u);
    (*p).m1[0] = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m1[1] = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p).m1[2] = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p).m1[3] = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
}
static int check_R69(const struct R69 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (long long)abi_mix(s, 0u);
    bad |= (*p).m1[0] != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[1] != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[2] != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[3] != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R70(struct R70 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (long long)abi_mix(s, 0u);
    (*p).m0.m1[0] = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m0.m1[1] = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p).m0.m1[2] = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p).m0.m1[3] = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p).m0.m2 = (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
}
static int check_R70(const struct R70 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (long long)abi_mix(s, 0u);
    bad |= (*p).m0.m1[0] != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m0.m1[1] != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p).m0.m1[2] != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p).m0.m1[3] != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p).m0.m2 != (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R71(struct R71 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (unsigned)(abi_mix(s, 1u) % 4294967296u);
    (*p).m1.m1 = (long long)abi_mix(s, 2u);
    (*p).m1.m2[0][0] = (long long)abi_mix(s, 3u);
    (*p).m1.m2[0][1] = (long long)abi_mix(s, 4u);
    (*p).m1.m2[1][0] = (long long)abi_mix(s, 5u);
    (*p).m1.m2[1][1] = (long long)abi_mix(s, 6u);
    (*p).m1.m2[2][0] = (long long)abi_mix(s, 7u);
    (*p).m1.m2[2][1] = (long long)abi_mix(s, 8u);
    (*p).m1.m2[3][0] = (long long)abi_mix(s, 9u);
    (*p).m1.m2[3][1] = (long long)abi_mix(s, 10u);
    (*p).m2 = (int)((long long)(abi_mix(s, 11u) % 4294967296u) - 2147483648);
    (*p).m3.m0[0] = (unsigned char)(abi_mix(s, 12u) % 256u);
    (*p).m3.m0[1] = (unsigned char)(abi_mix(s, 13u) % 256u);
    (*p).m3.m0[2] = (unsigned char)(abi_mix(s, 14u) % 256u);
    (*p).m3.m0[3] = (unsigned char)(abi_mix(s, 15u) % 256u);
    (*p).m3.m0[4] = (unsigned char)(abi_mix(s, 16u) % 256u);
    (*p).m3.m0[5] = (unsigned char)(abi_mix(s, 17u) % 256u);
    (*p).m3.m0[6] = (unsigned char)(abi_mix(s, 18u) % 256u);
    (*p).m3.m0[7] = (unsigned char)(abi_mix(s, 19u) % 256u);
    (*p).m3.m1 = (unsigned char)(abi_mix(s, 20u) % 256u);
    (*p).m3.m2 = (abi_anchor + (abi_mix(s, 21u) % 8u));
    (*p).m3.m3[0] = (float)((double)(long long)(abi_mix(s, 22u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m3[1] = (float)((double)(long long)(abi_mix(s, 23u) % 4001u) / 8.0 - 250.0);
    (*p).m4 = (unsigned)(abi_mix(s, 24u) % 4294967296u);
    (*p).m5.m0.m0 = (long long)abi_mix(s, 25u);
    (*p).m5.m0.m1[0] = (int)((long long)(abi_mix(s, 26u) % 4294967296u) - 2147483648);
    (*p).m5.m0.m1[1] = (int)((long long)(abi_mix(s, 27u) % 4294967296u) - 2147483648);
    (*p).m5.m0.m1[2] = (int)((long long)(abi_mix(s, 28u) % 4294967296u) - 2147483648);
    (*p).m5.m0.m1[3] = (int)((long long)(abi_mix(s, 29u) % 4294967296u) - 2147483648);
    (*p).m5.m0.m2 = (double)((double)(long long)(abi_mix(s, 30u) % 4001u) / 8.0 - 250.0);
}
static int check_R71(const struct R71 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (unsigned)(abi_mix(s, 1u) % 4294967296u);
    bad |= (*p).m1.m1 != (long long)abi_mix(s, 2u);
    bad |= (*p).m1.m2[0][0] != (long long)abi_mix(s, 3u);
    bad |= (*p).m1.m2[0][1] != (long long)abi_mix(s, 4u);
    bad |= (*p).m1.m2[1][0] != (long long)abi_mix(s, 5u);
    bad |= (*p).m1.m2[1][1] != (long long)abi_mix(s, 6u);
    bad |= (*p).m1.m2[2][0] != (long long)abi_mix(s, 7u);
    bad |= (*p).m1.m2[2][1] != (long long)abi_mix(s, 8u);
    bad |= (*p).m1.m2[3][0] != (long long)abi_mix(s, 9u);
    bad |= (*p).m1.m2[3][1] != (long long)abi_mix(s, 10u);
    bad |= (*p).m2 != (int)((long long)(abi_mix(s, 11u) % 4294967296u) - 2147483648);
    bad |= (*p).m3.m0[0] != (unsigned char)(abi_mix(s, 12u) % 256u);
    bad |= (*p).m3.m0[1] != (unsigned char)(abi_mix(s, 13u) % 256u);
    bad |= (*p).m3.m0[2] != (unsigned char)(abi_mix(s, 14u) % 256u);
    bad |= (*p).m3.m0[3] != (unsigned char)(abi_mix(s, 15u) % 256u);
    bad |= (*p).m3.m0[4] != (unsigned char)(abi_mix(s, 16u) % 256u);
    bad |= (*p).m3.m0[5] != (unsigned char)(abi_mix(s, 17u) % 256u);
    bad |= (*p).m3.m0[6] != (unsigned char)(abi_mix(s, 18u) % 256u);
    bad |= (*p).m3.m0[7] != (unsigned char)(abi_mix(s, 19u) % 256u);
    bad |= (*p).m3.m1 != (unsigned char)(abi_mix(s, 20u) % 256u);
    bad |= (*p).m3.m2 != (abi_anchor + (abi_mix(s, 21u) % 8u));
    bad |= (*p).m3.m3[0] != (float)((double)(long long)(abi_mix(s, 22u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m3[1] != (float)((double)(long long)(abi_mix(s, 23u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4 != (unsigned)(abi_mix(s, 24u) % 4294967296u);
    bad |= (*p).m5.m0.m0 != (long long)abi_mix(s, 25u);
    bad |= (*p).m5.m0.m1[0] != (int)((long long)(abi_mix(s, 26u) % 4294967296u) - 2147483648);
    bad |= (*p).m5.m0.m1[1] != (int)((long long)(abi_mix(s, 27u) % 4294967296u) - 2147483648);
    bad |= (*p).m5.m0.m1[2] != (int)((long long)(abi_mix(s, 28u) % 4294967296u) - 2147483648);
    bad |= (*p).m5.m0.m1[3] != (int)((long long)(abi_mix(s, 29u) % 4294967296u) - 2147483648);
    bad |= (*p).m5.m0.m2 != (double)((double)(long long)(abi_mix(s, 30u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R72(struct R72 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = ((_Float128)(long long)(abi_mix(s, 0u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(s, 0u) % 7u) * 0x1p-100f128);
    (*p).m1 = ((_Float128)(long long)(abi_mix(s, 1u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(s, 1u) % 7u) * 0x1p-100f128);
}
static int check_R72(const struct R72 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != ((_Float128)(long long)(abi_mix(s, 0u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(s, 0u) % 7u) * 0x1p-100f128);
    bad |= (*p).m1 != ((_Float128)(long long)(abi_mix(s, 1u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(s, 1u) % 7u) * 0x1p-100f128);
    return !bad;
}
static void fill_R73(struct R73 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = ((_Float128)(long long)(abi_mix(s, 0u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(s, 0u) % 7u) * 0x1p-100f128);
    (*p).m0.m1 = ((_Float128)(long long)(abi_mix(s, 1u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(s, 1u) % 7u) * 0x1p-100f128);
}
static int check_R73(const struct R73 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != ((_Float128)(long long)(abi_mix(s, 0u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(s, 0u) % 7u) * 0x1p-100f128);
    bad |= (*p).m0.m1 != ((_Float128)(long long)(abi_mix(s, 1u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(s, 1u) % 7u) * 0x1p-100f128);
    return !bad;
}
static void fill_R74(struct R74 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
    (*p).m1[0] = (unsigned long long)abi_mix(s, 1u);
    (*p).m1[1] = (unsigned long long)abi_mix(s, 2u);
    (*p).m1[2] = (unsigned long long)abi_mix(s, 3u);
}
static int check_R74(const struct R74 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    bad |= (*p).m1[0] != (unsigned long long)abi_mix(s, 1u);
    bad |= (*p).m1[1] != (unsigned long long)abi_mix(s, 2u);
    bad |= (*p).m1[2] != (unsigned long long)abi_mix(s, 3u);
    return !bad;
}
static void fill_R75(struct R75 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (_Float16)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R75(const struct R75 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (_Float16)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R76(struct R76 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (_Float16)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (_Float16)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R76(const struct R76 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (_Float16)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (_Float16)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (_Float16)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R77(struct R77 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m1[0][0] = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m1[0][1] = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p).m1[0][2] = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p).m1[0][3] = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p).m2[0] = (short)((long long)(abi_mix(s, 5u) % 65536u) - 32768);
    (*p).m2[1] = (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    (*p).m2[2] = (short)((long long)(abi_mix(s, 7u) % 65536u) - 32768);
    (*p).m2[3] = (short)((long long)(abi_mix(s, 8u) % 65536u) - 32768);
    (*p).m2[4] = (short)((long long)(abi_mix(s, 9u) % 65536u) - 32768);
    (*p).m2[5] = (short)((long long)(abi_mix(s, 10u) % 65536u) - 32768);
    (*p).m2[6] = (short)((long long)(abi_mix(s, 11u) % 65536u) - 32768);
    (*p).m2[7] = (short)((long long)(abi_mix(s, 12u) % 65536u) - 32768);
}
static int check_R77(const struct R77 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m1[0][0] != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[0][1] != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[0][2] != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[0][3] != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p).m2[0] != (short)((long long)(abi_mix(s, 5u) % 65536u) - 32768);
    bad |= (*p).m2[1] != (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    bad |= (*p).m2[2] != (short)((long long)(abi_mix(s, 7u) % 65536u) - 32768);
    bad |= (*p).m2[3] != (short)((long long)(abi_mix(s, 8u) % 65536u) - 32768);
    bad |= (*p).m2[4] != (short)((long long)(abi_mix(s, 9u) % 65536u) - 32768);
    bad |= (*p).m2[5] != (short)((long long)(abi_mix(s, 10u) % 65536u) - 32768);
    bad |= (*p).m2[6] != (short)((long long)(abi_mix(s, 11u) % 65536u) - 32768);
    bad |= (*p).m2[7] != (short)((long long)(abi_mix(s, 12u) % 65536u) - 32768);
    return !bad;
}
static void fill_R78(struct R78 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R78(const struct R78 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R79(struct R79 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m2 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R79(const struct R79 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m2 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R80(struct R80 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
}
static int check_R80(const struct R80 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    return !bad;
}

struct R0 abi_f0(struct R3 a0, ...);
unsigned long long abi_f1(struct R15 a0, struct R16 a1, union R17 a2, ...);
unsigned long long abi_f2(abi_v8uc a0, unsigned char a1, abi_v2ll a2);
void abi_f3(struct R31 a0, ...);
struct R42 abi_f4(int a0, abi_v2d a1, _Float16 a2, int a3, float a4, _Float128 a5, abi_v2d a6, struct R43 a7, _Float16 a8, abi_v4c a9, struct R44 a10, struct R47 a11, struct R48 a12, struct R49 a13, float a14);
struct R50 abi_f5(char * a0, struct R59 a1, struct R60 a2, struct R61 a3, ...);
struct R74 abi_f6(struct R76 a0, abi_v4f a1, struct R77 a2, struct R79 a3);
void abi_f7(double a0, int a1, struct R80 a2);
#endif
