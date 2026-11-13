/* abi_gen_105.h - written by gen_c_abi.py 105 8: the types of the
   tests in abi_gen_105.c and abi_gen_105_callee.c, and the functions that fill
   and check them. */
#ifndef ABI_GEN_105_H
#define ABI_GEN_105_H
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

struct R0 {
    char * m0[2];
    _Bool m1_0 : 1;
    unsigned m1_1 : 25;
    unsigned m1_2 : 15;
    unsigned m1_3 : 20;
    float m2;
};
struct R1 {
    _Decimal128 m0;
    _Decimal128 m1;
};
struct R2 {
    struct R1 m0;
};
struct R3 {
    _Decimal128 m0[3];
    _Alignas(16) char * m1;
};
union R4 {
    unsigned char m0;
    short m1;
};
struct R5 {
    union R4 m0;
    _Decimal128 m1;
    _Alignas(16) signed char m2;
};
struct R6 {
    struct R3 m0;
    struct R5 m1;
    unsigned long long m2;
    unsigned m3_0 : 3;
    int m3_1 : 18;
    _Alignas(64) short m4[4];
    _Decimal128 m5[5];
};
struct R7 {
    double m0;
    double m1;
};
struct R8 {
    unsigned long long m0;
    float m1[1];
    _Decimal64 m2;
    unsigned char m3;
    signed char m4;
};
union R9 {
    _Decimal128 m0;
    float m1;
    _Decimal128 m2;
    double m3;
    struct R8 m4;
    int m5;
    _Decimal32 m6;
    double m7;
};
struct R10 {
    int m0;
};
struct R11 {
    _Decimal64 m0;
    _Decimal64 m1;
    _Decimal64 m2;
    _Decimal64 m3;
};
struct R12 {
    struct R11 m0;
};
struct R13 {
    unsigned long long m0;
};
struct R14 {
    signed char m0;
};
struct R15 {
    _Decimal32 m0;
    struct R14 m1;
    _Decimal64 m2;
};
struct R16 {
    unsigned short m0;
    _Decimal32 m1;
};
struct R17 {
    _Bool m0_0 : 1;
    unsigned m0_1 : 1;
    int : 0;
    int m0_2 : 18;
    _Alignas(64) struct R15 m1;
    short m2;
    _Decimal32 m3;
    short m4;
    struct R16 m5[4];
};
union R18 {
    _Decimal64 m0;
    float m1[5];
};
struct R19 {
    _Decimal64 m0;
    union R18 m1;
};
struct R20 {
    _Decimal128 m0;
};
struct R21 {
    short m0[1];
};
struct R22 {
    struct R21 m0;
};
struct R23 {
    _Decimal64 m0;
};
struct R24 {
    _Decimal32 m0;
    unsigned char m1;
    _Decimal64 m2;
};
struct R25 {
    struct R24 m0;
};
struct R26 {
    long long m0[4];
    double m1;
    char * m2;
    unsigned short m3;
};
struct R27 {
    _Decimal128 m0;
    _Decimal128 m1;
};
struct R28 {
    unsigned long long m0;
    unsigned short m1;
    int m2_0 : 1;
    int m2_1 : 5;
    _Bool m2_2 : 1;
};
struct R29 {
    double m0;
    double m1;
};
struct R30 {
    double m0;
    struct R29 m1;
    double m2;
    double m3;
};
struct R31 {
    _Decimal64 m0;
    int m1_0 : 7;
    int m1_1 : 7;
    int m1_2 : 31;
    signed char m2;
};
struct R32 {
    unsigned long long m0[3];
    struct R31 m1;
    unsigned m2_0 : 29;
    int : 0;
    int m2_1 : 12;
    unsigned m2_2 : 27;
    _Bool m2_3 : 1;
};
union R33 {
    double m0;
    unsigned long long m1;
};
struct R34 {
    signed char m0;
    _Alignas(8) unsigned short m1;
    struct R32 m2;
    union R33 m3;
    int m4_0 : 2;
    _Bool m5_0 : 1;
    unsigned m5_1 : 5;
};
struct R35 {
    unsigned m0_0 : 22;
    unsigned m1_0 : 28;
    unsigned m1_1 : 30;
    int m1_2 : 22;
    _Bool m1_3 : 1;
    _Decimal32 m2;
    _Decimal32 m3[5];
};
struct R36 {
    float m0;
    float m1;
};
struct R37 {
    float m0;
};
union R38 {
    int m0;
    float m1;
    _Bool m2;
    unsigned m3;
    _Decimal32 m4[1];
};
struct R39 {
    unsigned m0_0 : 32;
    _Bool m0_1 : 1;
    _Bool m0_2 : 1;
    unsigned m1_0 : 13;
    _Bool m2;
};
struct R40 {
    _Bool m0_0 : 1;
    _Bool m0_1 : 1;
    int m0_2 : 12;
    int m0_3 : 9;
    unsigned short m1;
    unsigned long long m2;
    float m3;
    _Decimal64 m4;
    _Alignas(64) int m5;
    long long m6;
    unsigned m7;
};
struct R41 {
    _Decimal32 m0[2];
    _Decimal32 m1;
};
struct R42 {
    _Decimal32 m0[2];
    _Decimal32 m1;
    _Decimal32 m2;
    _Decimal32 m3;
};
struct R43 {
    _Decimal128 m0;
};
struct R44 {
    signed char m0;
    short m1;
    int m2_0 : 15;
    int m2_1 : 13;
    unsigned m2_2 : 4;
    unsigned m3;
    _Decimal128 m4;
};
struct R45 {
    _Decimal128 m0;
    struct R44 m1;
    long long m2;
    signed char m3[5];
    double m4[2];
};
struct R46 {
    int m0_0 : 20;
    _Alignas(64) _Decimal32 m1;
    struct R45 m2;
    int m3_0 : 14;
    unsigned m3_1 : 12;
    unsigned m3_2 : 28;
    int m3_3 : 12;
    _Decimal128 m4;
};
union R47 {
    _Decimal64 m0;
    float m1[5];
};
struct R48 {
    _Decimal64 m0;
};
struct R49 {
    _Decimal64 m0;
    _Decimal64 m1[2];
    struct R48 m2;
    _Decimal64 m3;
};
struct R50 {
    _Decimal32 m0;
};
struct R51 {
    _Decimal32 m0;
    _Bool m1_0 : 1;
    _Bool m1_1 : 1;
    int m2_0 : 15;
    int m2_1 : 8;
};
struct R52 {
    int m0_0 : 4;
    unsigned m0_1 : 2;
    unsigned m0_2 : 29;
    int m0_3 : 8;
};
struct R53 {
    unsigned char m0;
    _Alignas(8) unsigned short m1;
    float m2[1];
    struct R51 m3;
    struct R52 m4;
    _Decimal64 m5;
};
struct R54 {
    struct R53 m0;
    signed char m1;
    signed char m2;
    int m3;
    _Decimal32 m4;
    _Alignas(16) signed char m5;
};
struct R55 {
    int m0_0 : 15;
    short m1;
};
struct R56 {
    _Decimal64 m0;
    unsigned m1_0 : 22;
    unsigned m1_1 : 18;
};
struct R57 {
    _Decimal64 m0;
};
struct R58 {
    double m0;
    unsigned long long m1;
};
struct R59 {
    unsigned m0_0 : 26;
    int m0_1 : 26;
    _Bool m0_2 : 1;
    unsigned m0_3 : 11;
    long long m1;
    _Decimal32 m2;
};
union R60 {
    struct R58 m0;
    struct R59 m1;
};
struct R61 {
    _Bool m0_0 : 1;
    unsigned m0_1 : 23;
    unsigned m0_2 : 22;
    int : 3;
    _Bool m0_3 : 1;
    struct R57 m1;
    _Decimal32 m2;
    short m3;
    _Alignas(16) union R60 m4[1];
    unsigned m5_0 : 28;
    unsigned : 0;
    unsigned m5_1 : 20;
};
struct R62 {
    _Decimal32 m0;
};
struct R63 {
    double m0;
    double m1;
};
struct R64 {
    double m0;
    struct R63 m1;
    double m2;
    double m3;
};
struct R65 {
    double m0;
    double m1;
};
struct R66 {
    struct R65 m0;
};
struct R67 {
    _Decimal64 m0;
    _Alignas(16) double m1[2];
};
struct R68 {
    _Decimal128 m0;
    _Alignas(32) unsigned short m1[2];
    _Decimal128 m2;
};
struct R69 {
    _Decimal32 m0;
    _Decimal32 m1;
};
struct R70 {
    _Decimal128 m0;
    char * m1;
};
struct R71 {
    double m0[4];
    unsigned m1_0 : 2;
    struct R70 m2;
};
struct R72 {
    struct R71 m0;
    _Bool m1_0 : 1;
    _Bool m1_1 : 1;
    int m1_2 : 26;
    float m2;
    _Alignas(8) short m3;
    _Decimal128 m4;
};
struct R73 {
    double m0;
    double m1;
    double m2;
    double m3;
};
struct R74 {
    float m0;
    _Decimal32 m1;
    unsigned long long m2[1];
    _Decimal128 m3;
    int m4;
    _Decimal32 m5[3];
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
    (*p).m0 = (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R1(const struct R1 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R2(struct R2 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R2(const struct R2 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R3(struct R3 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2] = (_Decimal128)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (abi_anchor + (abi_mix(s, 3u) % 8u));
}
static int check_R3(const struct R3 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2] != (_Decimal128)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (abi_anchor + (abi_mix(s, 3u) % 8u));
    return !bad;
}
static void fill_R4(union R4 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
}
static int check_R4(const union R4 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    return !bad;
}
static void fill_R5(struct R5 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m1 = (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
}
static int check_R5(const struct R5 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m1 != (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    return !bad;
}
static void fill_R6(struct R6 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0[0] = (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m0[1] = (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m0[2] = (_Decimal128)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (abi_anchor + (abi_mix(s, 3u) % 8u));
    (*p).m1.m0.m0 = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p).m1.m1 = (_Decimal128)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2 = (signed char)((long long)(abi_mix(s, 6u) % 256u) - 128);
    (*p).m2 = (unsigned long long)abi_mix(s, 7u);
    (*p).m3_0 = (unsigned)(abi_mix(s, 8u) % 8u);
    (*p).m3_1 = (int)((long long)(abi_mix(s, 9u) % 262144u) - 131072);
    (*p).m4[0] = (short)((long long)(abi_mix(s, 10u) % 65536u) - 32768);
    (*p).m4[1] = (short)((long long)(abi_mix(s, 11u) % 65536u) - 32768);
    (*p).m4[2] = (short)((long long)(abi_mix(s, 12u) % 65536u) - 32768);
    (*p).m4[3] = (short)((long long)(abi_mix(s, 13u) % 65536u) - 32768);
    (*p).m5[0] = (_Decimal128)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    (*p).m5[1] = (_Decimal128)((double)(long long)(abi_mix(s, 15u) % 4001u) / 8.0 - 250.0);
    (*p).m5[2] = (_Decimal128)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
    (*p).m5[3] = (_Decimal128)((double)(long long)(abi_mix(s, 17u) % 4001u) / 8.0 - 250.0);
    (*p).m5[4] = (_Decimal128)((double)(long long)(abi_mix(s, 18u) % 4001u) / 8.0 - 250.0);
}
static int check_R6(const struct R6 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0[0] != (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m0[1] != (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m0[2] != (_Decimal128)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (abi_anchor + (abi_mix(s, 3u) % 8u));
    bad |= (*p).m1.m0.m0 != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p).m1.m1 != (_Decimal128)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2 != (signed char)((long long)(abi_mix(s, 6u) % 256u) - 128);
    bad |= (*p).m2 != (unsigned long long)abi_mix(s, 7u);
    bad |= (*p).m3_0 != (unsigned)(abi_mix(s, 8u) % 8u);
    bad |= (*p).m3_1 != (int)((long long)(abi_mix(s, 9u) % 262144u) - 131072);
    bad |= (*p).m4[0] != (short)((long long)(abi_mix(s, 10u) % 65536u) - 32768);
    bad |= (*p).m4[1] != (short)((long long)(abi_mix(s, 11u) % 65536u) - 32768);
    bad |= (*p).m4[2] != (short)((long long)(abi_mix(s, 12u) % 65536u) - 32768);
    bad |= (*p).m4[3] != (short)((long long)(abi_mix(s, 13u) % 65536u) - 32768);
    bad |= (*p).m5[0] != (_Decimal128)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5[1] != (_Decimal128)((double)(long long)(abi_mix(s, 15u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5[2] != (_Decimal128)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5[3] != (_Decimal128)((double)(long long)(abi_mix(s, 17u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5[4] != (_Decimal128)((double)(long long)(abi_mix(s, 18u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R7(struct R7 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R7(const struct R7 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R8(struct R8 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (unsigned char)(abi_mix(s, 3u) % 256u);
    (*p).m4 = (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
}
static int check_R8(const struct R8 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (unsigned char)(abi_mix(s, 3u) % 256u);
    bad |= (*p).m4 != (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
    return !bad;
}
static void fill_R9(union R9 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R9(const union R9 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R10(struct R10 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
}
static int check_R10(const struct R10 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R11(struct R11 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (_Decimal64)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (_Decimal64)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R11(const struct R11 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (_Decimal64)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (_Decimal64)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R12(struct R12 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (_Decimal64)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m2 = (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m3 = (_Decimal64)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R12(const struct R12 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (_Decimal64)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m2 != (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m3 != (_Decimal64)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R13(struct R13 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
}
static int check_R13(const struct R13 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    return !bad;
}
static void fill_R14(struct R14 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
}
static int check_R14(const struct R14 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    return !bad;
}
static void fill_R15(struct R15 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m2 = (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R15(const struct R15 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m2 != (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R16(struct R16 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m1 = (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R16(const struct R16 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m1 != (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R17(struct R17 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) & 1u);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 262144u) - 131072);
    (*p).m1.m0 = (_Decimal32)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1.m0 = (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
    (*p).m1.m2 = (_Decimal64)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    (*p).m3 = (_Decimal32)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m4 = (short)((long long)(abi_mix(s, 8u) % 65536u) - 32768);
    (*p).m5[0].m0 = (unsigned short)(abi_mix(s, 9u) % 65536u);
    (*p).m5[0].m1 = (_Decimal32)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    (*p).m5[1].m0 = (unsigned short)(abi_mix(s, 11u) % 65536u);
    (*p).m5[1].m1 = (_Decimal32)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    (*p).m5[2].m0 = (unsigned short)(abi_mix(s, 13u) % 65536u);
    (*p).m5[2].m1 = (_Decimal32)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    (*p).m5[3].m0 = (unsigned short)(abi_mix(s, 15u) % 65536u);
    (*p).m5[3].m1 = (_Decimal32)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
}
static int check_R17(const struct R17 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 262144u) - 131072);
    bad |= (*p).m1.m0 != (_Decimal32)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1.m0 != (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
    bad |= (*p).m1.m2 != (_Decimal64)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    bad |= (*p).m3 != (_Decimal32)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4 != (short)((long long)(abi_mix(s, 8u) % 65536u) - 32768);
    bad |= (*p).m5[0].m0 != (unsigned short)(abi_mix(s, 9u) % 65536u);
    bad |= (*p).m5[0].m1 != (_Decimal32)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5[1].m0 != (unsigned short)(abi_mix(s, 11u) % 65536u);
    bad |= (*p).m5[1].m1 != (_Decimal32)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5[2].m0 != (unsigned short)(abi_mix(s, 13u) % 65536u);
    bad |= (*p).m5[2].m1 != (_Decimal32)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5[3].m0 != (unsigned short)(abi_mix(s, 15u) % 65536u);
    bad |= (*p).m5[3].m1 != (_Decimal32)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R18(union R18 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R18(const union R18 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R19(struct R19 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (_Decimal64)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R19(const struct R19 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (_Decimal64)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R20(struct R20 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R20(const struct R20 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R21(struct R21 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
}
static int check_R21(const struct R21 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    return !bad;
}
static void fill_R22(struct R22 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0[0] = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
}
static int check_R22(const struct R22 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0[0] != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    return !bad;
}
static void fill_R23(struct R23 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R23(const struct R23 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R24(struct R24 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m2 = (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R24(const struct R24 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m2 != (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R25(struct R25 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m0.m2 = (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R25(const struct R25 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m0.m2 != (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R26(struct R26 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (long long)abi_mix(s, 0u);
    (*p).m0[1] = (long long)abi_mix(s, 1u);
    (*p).m0[2] = (long long)abi_mix(s, 2u);
    (*p).m0[3] = (long long)abi_mix(s, 3u);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (abi_anchor + (abi_mix(s, 5u) % 8u));
    (*p).m3 = (unsigned short)(abi_mix(s, 6u) % 65536u);
}
static int check_R26(const struct R26 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (long long)abi_mix(s, 0u);
    bad |= (*p).m0[1] != (long long)abi_mix(s, 1u);
    bad |= (*p).m0[2] != (long long)abi_mix(s, 2u);
    bad |= (*p).m0[3] != (long long)abi_mix(s, 3u);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (abi_anchor + (abi_mix(s, 5u) % 8u));
    bad |= (*p).m3 != (unsigned short)(abi_mix(s, 6u) % 65536u);
    return !bad;
}
static void fill_R27(struct R27 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R27(const struct R27 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (_Decimal128)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R28(struct R28 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
    (*p).m1 = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m2_0 = (int)((long long)(abi_mix(s, 2u) % 2u) - 1);
    (*p).m2_1 = (int)((long long)(abi_mix(s, 3u) % 32u) - 16);
    (*p).m2_2 = (_Bool)(abi_mix(s, 4u) & 1u);
}
static int check_R28(const struct R28 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m1 != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m2_0 != (int)((long long)(abi_mix(s, 2u) % 2u) - 1);
    bad |= (*p).m2_1 != (int)((long long)(abi_mix(s, 3u) % 32u) - 16);
    bad |= (*p).m2_2 != (_Bool)(abi_mix(s, 4u) & 1u);
    return !bad;
}
static void fill_R29(struct R29 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R29(const struct R29 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R30(struct R30 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R30(const struct R30 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R31(struct R31 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1_0 = (int)((long long)(abi_mix(s, 1u) % 128u) - 64);
    (*p).m1_1 = (int)((long long)(abi_mix(s, 2u) % 128u) - 64);
    (*p).m1_2 = (int)((long long)(abi_mix(s, 3u) % 2147483648u) - 1073741824);
    (*p).m2 = (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
}
static int check_R31(const struct R31 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1_0 != (int)((long long)(abi_mix(s, 1u) % 128u) - 64);
    bad |= (*p).m1_1 != (int)((long long)(abi_mix(s, 2u) % 128u) - 64);
    bad |= (*p).m1_2 != (int)((long long)(abi_mix(s, 3u) % 2147483648u) - 1073741824);
    bad |= (*p).m2 != (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
    return !bad;
}
static void fill_R32(struct R32 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned long long)abi_mix(s, 0u);
    (*p).m0[1] = (unsigned long long)abi_mix(s, 1u);
    (*p).m0[2] = (unsigned long long)abi_mix(s, 2u);
    (*p).m1.m0 = (_Decimal64)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1_0 = (int)((long long)(abi_mix(s, 4u) % 128u) - 64);
    (*p).m1.m1_1 = (int)((long long)(abi_mix(s, 5u) % 128u) - 64);
    (*p).m1.m1_2 = (int)((long long)(abi_mix(s, 6u) % 2147483648u) - 1073741824);
    (*p).m1.m2 = (signed char)((long long)(abi_mix(s, 7u) % 256u) - 128);
    (*p).m2_0 = (unsigned)(abi_mix(s, 8u) % 536870912u);
    (*p).m2_1 = (int)((long long)(abi_mix(s, 9u) % 4096u) - 2048);
    (*p).m2_2 = (unsigned)(abi_mix(s, 10u) % 134217728u);
    (*p).m2_3 = (_Bool)(abi_mix(s, 11u) & 1u);
}
static int check_R32(const struct R32 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m0[1] != (unsigned long long)abi_mix(s, 1u);
    bad |= (*p).m0[2] != (unsigned long long)abi_mix(s, 2u);
    bad |= (*p).m1.m0 != (_Decimal64)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1_0 != (int)((long long)(abi_mix(s, 4u) % 128u) - 64);
    bad |= (*p).m1.m1_1 != (int)((long long)(abi_mix(s, 5u) % 128u) - 64);
    bad |= (*p).m1.m1_2 != (int)((long long)(abi_mix(s, 6u) % 2147483648u) - 1073741824);
    bad |= (*p).m1.m2 != (signed char)((long long)(abi_mix(s, 7u) % 256u) - 128);
    bad |= (*p).m2_0 != (unsigned)(abi_mix(s, 8u) % 536870912u);
    bad |= (*p).m2_1 != (int)((long long)(abi_mix(s, 9u) % 4096u) - 2048);
    bad |= (*p).m2_2 != (unsigned)(abi_mix(s, 10u) % 134217728u);
    bad |= (*p).m2_3 != (_Bool)(abi_mix(s, 11u) & 1u);
    return !bad;
}
static void fill_R33(union R33 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R33(const union R33 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R34(struct R34 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m1 = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m2.m0[0] = (unsigned long long)abi_mix(s, 2u);
    (*p).m2.m0[1] = (unsigned long long)abi_mix(s, 3u);
    (*p).m2.m0[2] = (unsigned long long)abi_mix(s, 4u);
    (*p).m2.m1.m0 = (_Decimal64)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m1.m1_0 = (int)((long long)(abi_mix(s, 6u) % 128u) - 64);
    (*p).m2.m1.m1_1 = (int)((long long)(abi_mix(s, 7u) % 128u) - 64);
    (*p).m2.m1.m1_2 = (int)((long long)(abi_mix(s, 8u) % 2147483648u) - 1073741824);
    (*p).m2.m1.m2 = (signed char)((long long)(abi_mix(s, 9u) % 256u) - 128);
    (*p).m2.m2_0 = (unsigned)(abi_mix(s, 10u) % 536870912u);
    (*p).m2.m2_1 = (int)((long long)(abi_mix(s, 11u) % 4096u) - 2048);
    (*p).m2.m2_2 = (unsigned)(abi_mix(s, 12u) % 134217728u);
    (*p).m2.m2_3 = (_Bool)(abi_mix(s, 13u) & 1u);
    (*p).m3.m0 = (double)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    (*p).m4_0 = (int)((long long)(abi_mix(s, 15u) % 4u) - 2);
    (*p).m5_0 = (_Bool)(abi_mix(s, 16u) & 1u);
    (*p).m5_1 = (unsigned)(abi_mix(s, 17u) % 32u);
}
static int check_R34(const struct R34 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m1 != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m2.m0[0] != (unsigned long long)abi_mix(s, 2u);
    bad |= (*p).m2.m0[1] != (unsigned long long)abi_mix(s, 3u);
    bad |= (*p).m2.m0[2] != (unsigned long long)abi_mix(s, 4u);
    bad |= (*p).m2.m1.m0 != (_Decimal64)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m1.m1_0 != (int)((long long)(abi_mix(s, 6u) % 128u) - 64);
    bad |= (*p).m2.m1.m1_1 != (int)((long long)(abi_mix(s, 7u) % 128u) - 64);
    bad |= (*p).m2.m1.m1_2 != (int)((long long)(abi_mix(s, 8u) % 2147483648u) - 1073741824);
    bad |= (*p).m2.m1.m2 != (signed char)((long long)(abi_mix(s, 9u) % 256u) - 128);
    bad |= (*p).m2.m2_0 != (unsigned)(abi_mix(s, 10u) % 536870912u);
    bad |= (*p).m2.m2_1 != (int)((long long)(abi_mix(s, 11u) % 4096u) - 2048);
    bad |= (*p).m2.m2_2 != (unsigned)(abi_mix(s, 12u) % 134217728u);
    bad |= (*p).m2.m2_3 != (_Bool)(abi_mix(s, 13u) & 1u);
    bad |= (*p).m3.m0 != (double)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4_0 != (int)((long long)(abi_mix(s, 15u) % 4u) - 2);
    bad |= (*p).m5_0 != (_Bool)(abi_mix(s, 16u) & 1u);
    bad |= (*p).m5_1 != (unsigned)(abi_mix(s, 17u) % 32u);
    return !bad;
}
static void fill_R35(struct R35 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 4194304u);
    (*p).m1_0 = (unsigned)(abi_mix(s, 1u) % 268435456u);
    (*p).m1_1 = (unsigned)(abi_mix(s, 2u) % 1073741824u);
    (*p).m1_2 = (int)((long long)(abi_mix(s, 3u) % 4194304u) - 2097152);
    (*p).m1_3 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m2 = (_Decimal32)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m3[0] = (_Decimal32)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1] = (_Decimal32)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m3[2] = (_Decimal32)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    (*p).m3[3] = (_Decimal32)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    (*p).m3[4] = (_Decimal32)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
}
static int check_R35(const struct R35 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 4194304u);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 1u) % 268435456u);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 2u) % 1073741824u);
    bad |= (*p).m1_2 != (int)((long long)(abi_mix(s, 3u) % 4194304u) - 2097152);
    bad |= (*p).m1_3 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m2 != (_Decimal32)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[0] != (_Decimal32)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1] != (_Decimal32)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[2] != (_Decimal32)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[3] != (_Decimal32)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[4] != (_Decimal32)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R36(struct R36 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R36(const struct R36 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R37(struct R37 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R37(const struct R37 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R38(union R38 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
}
static int check_R38(const union R38 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R39(struct R39 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
    (*p).m0_1 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m0_2 = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m1_0 = (unsigned)(abi_mix(s, 3u) % 8192u);
    (*p).m2 = (_Bool)(abi_mix(s, 4u) & 1u);
}
static int check_R39(const struct R39 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    bad |= (*p).m0_1 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m0_2 != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 3u) % 8192u);
    bad |= (*p).m2 != (_Bool)(abi_mix(s, 4u) & 1u);
    return !bad;
}
static void fill_R40(struct R40 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m0_1 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 4096u) - 2048);
    (*p).m0_3 = (int)((long long)(abi_mix(s, 3u) % 512u) - 256);
    (*p).m1 = (unsigned short)(abi_mix(s, 4u) % 65536u);
    (*p).m2 = (unsigned long long)abi_mix(s, 5u);
    (*p).m3 = (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m4 = (_Decimal64)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m5 = (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    (*p).m6 = (long long)abi_mix(s, 9u);
    (*p).m7 = (unsigned)(abi_mix(s, 10u) % 4294967296u);
}
static int check_R40(const struct R40 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m0_1 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 4096u) - 2048);
    bad |= (*p).m0_3 != (int)((long long)(abi_mix(s, 3u) % 512u) - 256);
    bad |= (*p).m1 != (unsigned short)(abi_mix(s, 4u) % 65536u);
    bad |= (*p).m2 != (unsigned long long)abi_mix(s, 5u);
    bad |= (*p).m3 != (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4 != (_Decimal64)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5 != (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    bad |= (*p).m6 != (long long)abi_mix(s, 9u);
    bad |= (*p).m7 != (unsigned)(abi_mix(s, 10u) % 4294967296u);
    return !bad;
}
static void fill_R41(struct R41 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (_Decimal32)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R41(const struct R41 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (_Decimal32)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R42(struct R42 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (_Decimal32)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (_Decimal32)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (_Decimal32)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R42(const struct R42 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (_Decimal32)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (_Decimal32)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (_Decimal32)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R43(struct R43 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R43(const struct R43 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R44(struct R44 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m1 = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    (*p).m2_0 = (int)((long long)(abi_mix(s, 2u) % 32768u) - 16384);
    (*p).m2_1 = (int)((long long)(abi_mix(s, 3u) % 8192u) - 4096);
    (*p).m2_2 = (unsigned)(abi_mix(s, 4u) % 16u);
    (*p).m3 = (unsigned)(abi_mix(s, 5u) % 4294967296u);
    (*p).m4 = (_Decimal128)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
}
static int check_R44(const struct R44 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m1 != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    bad |= (*p).m2_0 != (int)((long long)(abi_mix(s, 2u) % 32768u) - 16384);
    bad |= (*p).m2_1 != (int)((long long)(abi_mix(s, 3u) % 8192u) - 4096);
    bad |= (*p).m2_2 != (unsigned)(abi_mix(s, 4u) % 16u);
    bad |= (*p).m3 != (unsigned)(abi_mix(s, 5u) % 4294967296u);
    bad |= (*p).m4 != (_Decimal128)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R45(struct R45 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m1.m1 = (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    (*p).m1.m2_0 = (int)((long long)(abi_mix(s, 3u) % 32768u) - 16384);
    (*p).m1.m2_1 = (int)((long long)(abi_mix(s, 4u) % 8192u) - 4096);
    (*p).m1.m2_2 = (unsigned)(abi_mix(s, 5u) % 16u);
    (*p).m1.m3 = (unsigned)(abi_mix(s, 6u) % 4294967296u);
    (*p).m1.m4 = (_Decimal128)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (long long)abi_mix(s, 8u);
    (*p).m3[0] = (signed char)((long long)(abi_mix(s, 9u) % 256u) - 128);
    (*p).m3[1] = (signed char)((long long)(abi_mix(s, 10u) % 256u) - 128);
    (*p).m3[2] = (signed char)((long long)(abi_mix(s, 11u) % 256u) - 128);
    (*p).m3[3] = (signed char)((long long)(abi_mix(s, 12u) % 256u) - 128);
    (*p).m3[4] = (signed char)((long long)(abi_mix(s, 13u) % 256u) - 128);
    (*p).m4[0] = (double)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    (*p).m4[1] = (double)((double)(long long)(abi_mix(s, 15u) % 4001u) / 8.0 - 250.0);
}
static int check_R45(const struct R45 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m1.m1 != (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    bad |= (*p).m1.m2_0 != (int)((long long)(abi_mix(s, 3u) % 32768u) - 16384);
    bad |= (*p).m1.m2_1 != (int)((long long)(abi_mix(s, 4u) % 8192u) - 4096);
    bad |= (*p).m1.m2_2 != (unsigned)(abi_mix(s, 5u) % 16u);
    bad |= (*p).m1.m3 != (unsigned)(abi_mix(s, 6u) % 4294967296u);
    bad |= (*p).m1.m4 != (_Decimal128)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (long long)abi_mix(s, 8u);
    bad |= (*p).m3[0] != (signed char)((long long)(abi_mix(s, 9u) % 256u) - 128);
    bad |= (*p).m3[1] != (signed char)((long long)(abi_mix(s, 10u) % 256u) - 128);
    bad |= (*p).m3[2] != (signed char)((long long)(abi_mix(s, 11u) % 256u) - 128);
    bad |= (*p).m3[3] != (signed char)((long long)(abi_mix(s, 12u) % 256u) - 128);
    bad |= (*p).m3[4] != (signed char)((long long)(abi_mix(s, 13u) % 256u) - 128);
    bad |= (*p).m4[0] != (double)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[1] != (double)((double)(long long)(abi_mix(s, 15u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R46(struct R46 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 1048576u) - 524288);
    (*p).m1 = (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m0 = (_Decimal128)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m1.m0 = (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    (*p).m2.m1.m1 = (short)((long long)(abi_mix(s, 4u) % 65536u) - 32768);
    (*p).m2.m1.m2_0 = (int)((long long)(abi_mix(s, 5u) % 32768u) - 16384);
    (*p).m2.m1.m2_1 = (int)((long long)(abi_mix(s, 6u) % 8192u) - 4096);
    (*p).m2.m1.m2_2 = (unsigned)(abi_mix(s, 7u) % 16u);
    (*p).m2.m1.m3 = (unsigned)(abi_mix(s, 8u) % 4294967296u);
    (*p).m2.m1.m4 = (_Decimal128)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m2 = (long long)abi_mix(s, 10u);
    (*p).m2.m3[0] = (signed char)((long long)(abi_mix(s, 11u) % 256u) - 128);
    (*p).m2.m3[1] = (signed char)((long long)(abi_mix(s, 12u) % 256u) - 128);
    (*p).m2.m3[2] = (signed char)((long long)(abi_mix(s, 13u) % 256u) - 128);
    (*p).m2.m3[3] = (signed char)((long long)(abi_mix(s, 14u) % 256u) - 128);
    (*p).m2.m3[4] = (signed char)((long long)(abi_mix(s, 15u) % 256u) - 128);
    (*p).m2.m4[0] = (double)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m4[1] = (double)((double)(long long)(abi_mix(s, 17u) % 4001u) / 8.0 - 250.0);
    (*p).m3_0 = (int)((long long)(abi_mix(s, 18u) % 16384u) - 8192);
    (*p).m3_1 = (unsigned)(abi_mix(s, 19u) % 4096u);
    (*p).m3_2 = (unsigned)(abi_mix(s, 20u) % 268435456u);
    (*p).m3_3 = (int)((long long)(abi_mix(s, 21u) % 4096u) - 2048);
    (*p).m4 = (_Decimal128)((double)(long long)(abi_mix(s, 22u) % 4001u) / 8.0 - 250.0);
}
static int check_R46(const struct R46 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 1048576u) - 524288);
    bad |= (*p).m1 != (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m0 != (_Decimal128)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m1.m0 != (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    bad |= (*p).m2.m1.m1 != (short)((long long)(abi_mix(s, 4u) % 65536u) - 32768);
    bad |= (*p).m2.m1.m2_0 != (int)((long long)(abi_mix(s, 5u) % 32768u) - 16384);
    bad |= (*p).m2.m1.m2_1 != (int)((long long)(abi_mix(s, 6u) % 8192u) - 4096);
    bad |= (*p).m2.m1.m2_2 != (unsigned)(abi_mix(s, 7u) % 16u);
    bad |= (*p).m2.m1.m3 != (unsigned)(abi_mix(s, 8u) % 4294967296u);
    bad |= (*p).m2.m1.m4 != (_Decimal128)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m2 != (long long)abi_mix(s, 10u);
    bad |= (*p).m2.m3[0] != (signed char)((long long)(abi_mix(s, 11u) % 256u) - 128);
    bad |= (*p).m2.m3[1] != (signed char)((long long)(abi_mix(s, 12u) % 256u) - 128);
    bad |= (*p).m2.m3[2] != (signed char)((long long)(abi_mix(s, 13u) % 256u) - 128);
    bad |= (*p).m2.m3[3] != (signed char)((long long)(abi_mix(s, 14u) % 256u) - 128);
    bad |= (*p).m2.m3[4] != (signed char)((long long)(abi_mix(s, 15u) % 256u) - 128);
    bad |= (*p).m2.m4[0] != (double)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m4[1] != (double)((double)(long long)(abi_mix(s, 17u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3_0 != (int)((long long)(abi_mix(s, 18u) % 16384u) - 8192);
    bad |= (*p).m3_1 != (unsigned)(abi_mix(s, 19u) % 4096u);
    bad |= (*p).m3_2 != (unsigned)(abi_mix(s, 20u) % 268435456u);
    bad |= (*p).m3_3 != (int)((long long)(abi_mix(s, 21u) % 4096u) - 2048);
    bad |= (*p).m4 != (_Decimal128)((double)(long long)(abi_mix(s, 22u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R47(union R47 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R47(const union R47 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R48(struct R48 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R48(const struct R48 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R49(struct R49 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (_Decimal64)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m0 = (_Decimal64)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (_Decimal64)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R49(const struct R49 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (_Decimal64)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (_Decimal64)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m0 != (_Decimal64)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (_Decimal64)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R50(struct R50 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R50(const struct R50 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R51(struct R51 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1_0 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m1_1 = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m2_0 = (int)((long long)(abi_mix(s, 3u) % 32768u) - 16384);
    (*p).m2_1 = (int)((long long)(abi_mix(s, 4u) % 256u) - 128);
}
static int check_R51(const struct R51 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1_0 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m1_1 != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m2_0 != (int)((long long)(abi_mix(s, 3u) % 32768u) - 16384);
    bad |= (*p).m2_1 != (int)((long long)(abi_mix(s, 4u) % 256u) - 128);
    return !bad;
}
static void fill_R52(struct R52 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 16u) - 8);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 4u);
    (*p).m0_2 = (unsigned)(abi_mix(s, 2u) % 536870912u);
    (*p).m0_3 = (int)((long long)(abi_mix(s, 3u) % 256u) - 128);
}
static int check_R52(const struct R52 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 16u) - 8);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 4u);
    bad |= (*p).m0_2 != (unsigned)(abi_mix(s, 2u) % 536870912u);
    bad |= (*p).m0_3 != (int)((long long)(abi_mix(s, 3u) % 256u) - 128);
    return !bad;
}
static void fill_R53(struct R53 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m1 = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m2[0] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0 = (_Decimal32)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m1_0 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m3.m1_1 = (_Bool)(abi_mix(s, 5u) & 1u);
    (*p).m3.m2_0 = (int)((long long)(abi_mix(s, 6u) % 32768u) - 16384);
    (*p).m3.m2_1 = (int)((long long)(abi_mix(s, 7u) % 256u) - 128);
    (*p).m4.m0_0 = (int)((long long)(abi_mix(s, 8u) % 16u) - 8);
    (*p).m4.m0_1 = (unsigned)(abi_mix(s, 9u) % 4u);
    (*p).m4.m0_2 = (unsigned)(abi_mix(s, 10u) % 536870912u);
    (*p).m4.m0_3 = (int)((long long)(abi_mix(s, 11u) % 256u) - 128);
    (*p).m5 = (_Decimal64)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
}
static int check_R53(const struct R53 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m1 != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m2[0] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0 != (_Decimal32)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m1_0 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m3.m1_1 != (_Bool)(abi_mix(s, 5u) & 1u);
    bad |= (*p).m3.m2_0 != (int)((long long)(abi_mix(s, 6u) % 32768u) - 16384);
    bad |= (*p).m3.m2_1 != (int)((long long)(abi_mix(s, 7u) % 256u) - 128);
    bad |= (*p).m4.m0_0 != (int)((long long)(abi_mix(s, 8u) % 16u) - 8);
    bad |= (*p).m4.m0_1 != (unsigned)(abi_mix(s, 9u) % 4u);
    bad |= (*p).m4.m0_2 != (unsigned)(abi_mix(s, 10u) % 536870912u);
    bad |= (*p).m4.m0_3 != (int)((long long)(abi_mix(s, 11u) % 256u) - 128);
    bad |= (*p).m5 != (_Decimal64)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R54(struct R54 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m0.m1 = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m0.m2[0] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m3.m0 = (_Decimal32)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m3.m1_0 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m0.m3.m1_1 = (_Bool)(abi_mix(s, 5u) & 1u);
    (*p).m0.m3.m2_0 = (int)((long long)(abi_mix(s, 6u) % 32768u) - 16384);
    (*p).m0.m3.m2_1 = (int)((long long)(abi_mix(s, 7u) % 256u) - 128);
    (*p).m0.m4.m0_0 = (int)((long long)(abi_mix(s, 8u) % 16u) - 8);
    (*p).m0.m4.m0_1 = (unsigned)(abi_mix(s, 9u) % 4u);
    (*p).m0.m4.m0_2 = (unsigned)(abi_mix(s, 10u) % 536870912u);
    (*p).m0.m4.m0_3 = (int)((long long)(abi_mix(s, 11u) % 256u) - 128);
    (*p).m0.m5 = (_Decimal64)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (signed char)((long long)(abi_mix(s, 13u) % 256u) - 128);
    (*p).m2 = (signed char)((long long)(abi_mix(s, 14u) % 256u) - 128);
    (*p).m3 = (int)((long long)(abi_mix(s, 15u) % 4294967296u) - 2147483648);
    (*p).m4 = (_Decimal32)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
    (*p).m5 = (signed char)((long long)(abi_mix(s, 17u) % 256u) - 128);
}
static int check_R54(const struct R54 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m0.m1 != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m0.m2[0] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m3.m0 != (_Decimal32)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m3.m1_0 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m0.m3.m1_1 != (_Bool)(abi_mix(s, 5u) & 1u);
    bad |= (*p).m0.m3.m2_0 != (int)((long long)(abi_mix(s, 6u) % 32768u) - 16384);
    bad |= (*p).m0.m3.m2_1 != (int)((long long)(abi_mix(s, 7u) % 256u) - 128);
    bad |= (*p).m0.m4.m0_0 != (int)((long long)(abi_mix(s, 8u) % 16u) - 8);
    bad |= (*p).m0.m4.m0_1 != (unsigned)(abi_mix(s, 9u) % 4u);
    bad |= (*p).m0.m4.m0_2 != (unsigned)(abi_mix(s, 10u) % 536870912u);
    bad |= (*p).m0.m4.m0_3 != (int)((long long)(abi_mix(s, 11u) % 256u) - 128);
    bad |= (*p).m0.m5 != (_Decimal64)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (signed char)((long long)(abi_mix(s, 13u) % 256u) - 128);
    bad |= (*p).m2 != (signed char)((long long)(abi_mix(s, 14u) % 256u) - 128);
    bad |= (*p).m3 != (int)((long long)(abi_mix(s, 15u) % 4294967296u) - 2147483648);
    bad |= (*p).m4 != (_Decimal32)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5 != (signed char)((long long)(abi_mix(s, 17u) % 256u) - 128);
    return !bad;
}
static void fill_R55(struct R55 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 32768u) - 16384);
    (*p).m1 = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
}
static int check_R55(const struct R55 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 32768u) - 16384);
    bad |= (*p).m1 != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    return !bad;
}
static void fill_R56(struct R56 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1_0 = (unsigned)(abi_mix(s, 1u) % 4194304u);
    (*p).m1_1 = (unsigned)(abi_mix(s, 2u) % 262144u);
}
static int check_R56(const struct R56 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 1u) % 4194304u);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 2u) % 262144u);
    return !bad;
}
static void fill_R57(struct R57 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R57(const struct R57 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R58(struct R58 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (unsigned long long)abi_mix(s, 1u);
}
static int check_R58(const struct R58 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (unsigned long long)abi_mix(s, 1u);
    return !bad;
}
static void fill_R59(struct R59 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 67108864u);
    (*p).m0_1 = (int)((long long)(abi_mix(s, 1u) % 67108864u) - 33554432);
    (*p).m0_2 = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m0_3 = (unsigned)(abi_mix(s, 3u) % 2048u);
    (*p).m1 = (long long)abi_mix(s, 4u);
    (*p).m2 = (_Decimal32)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
}
static int check_R59(const struct R59 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 67108864u);
    bad |= (*p).m0_1 != (int)((long long)(abi_mix(s, 1u) % 67108864u) - 33554432);
    bad |= (*p).m0_2 != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m0_3 != (unsigned)(abi_mix(s, 3u) % 2048u);
    bad |= (*p).m1 != (long long)abi_mix(s, 4u);
    bad |= (*p).m2 != (_Decimal32)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R60(union R60 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (unsigned long long)abi_mix(s, 1u);
}
static int check_R60(const union R60 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (unsigned long long)abi_mix(s, 1u);
    return !bad;
}
static void fill_R61(struct R61 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 8388608u);
    (*p).m0_2 = (unsigned)(abi_mix(s, 2u) % 4194304u);
    (*p).m0_3 = (_Bool)(abi_mix(s, 3u) & 1u);
    (*p).m1.m0 = (_Decimal64)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (_Decimal32)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    (*p).m4[0].m0.m0 = (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m4[0].m0.m1 = (unsigned long long)abi_mix(s, 8u);
    (*p).m5_0 = (unsigned)(abi_mix(s, 9u) % 268435456u);
    (*p).m5_1 = (unsigned)(abi_mix(s, 10u) % 1048576u);
}
static int check_R61(const struct R61 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 8388608u);
    bad |= (*p).m0_2 != (unsigned)(abi_mix(s, 2u) % 4194304u);
    bad |= (*p).m0_3 != (_Bool)(abi_mix(s, 3u) & 1u);
    bad |= (*p).m1.m0 != (_Decimal64)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (_Decimal32)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (short)((long long)(abi_mix(s, 6u) % 65536u) - 32768);
    bad |= (*p).m4[0].m0.m0 != (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[0].m0.m1 != (unsigned long long)abi_mix(s, 8u);
    bad |= (*p).m5_0 != (unsigned)(abi_mix(s, 9u) % 268435456u);
    bad |= (*p).m5_1 != (unsigned)(abi_mix(s, 10u) % 1048576u);
    return !bad;
}
static void fill_R62(struct R62 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R62(const struct R62 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R63(struct R63 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R63(const struct R63 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R64(struct R64 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R64(const struct R64 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R65(struct R65 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R65(const struct R65 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R66(struct R66 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R66(const struct R66 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R67(struct R67 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R67(const struct R67 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal64)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R68(struct R68 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m1[1] = (unsigned short)(abi_mix(s, 2u) % 65536u);
    (*p).m2 = (_Decimal128)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R68(const struct R68 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m1[1] != (unsigned short)(abi_mix(s, 2u) % 65536u);
    bad |= (*p).m2 != (_Decimal128)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R69(struct R69 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R69(const struct R69 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal32)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R70(struct R70 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (abi_anchor + (abi_mix(s, 1u) % 8u));
}
static int check_R70(const struct R70 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Decimal128)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (abi_anchor + (abi_mix(s, 1u) % 8u));
    return !bad;
}
static void fill_R71(struct R71 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1_0 = (unsigned)(abi_mix(s, 4u) % 4u);
    (*p).m2.m0 = (_Decimal128)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m1 = (abi_anchor + (abi_mix(s, 6u) % 8u));
}
static int check_R71(const struct R71 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 4u) % 4u);
    bad |= (*p).m2.m0 != (_Decimal128)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m1 != (abi_anchor + (abi_mix(s, 6u) % 8u));
    return !bad;
}
static void fill_R72(struct R72 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m0[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m0[2] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m0[3] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1_0 = (unsigned)(abi_mix(s, 4u) % 4u);
    (*p).m0.m2.m0 = (_Decimal128)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m2.m1 = (abi_anchor + (abi_mix(s, 6u) % 8u));
    (*p).m1_0 = (_Bool)(abi_mix(s, 7u) & 1u);
    (*p).m1_1 = (_Bool)(abi_mix(s, 8u) & 1u);
    (*p).m1_2 = (int)((long long)(abi_mix(s, 9u) % 67108864u) - 33554432);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (short)((long long)(abi_mix(s, 11u) % 65536u) - 32768);
    (*p).m4 = (_Decimal128)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
}
static int check_R72(const struct R72 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m0[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m0[2] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m0[3] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1_0 != (unsigned)(abi_mix(s, 4u) % 4u);
    bad |= (*p).m0.m2.m0 != (_Decimal128)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m2.m1 != (abi_anchor + (abi_mix(s, 6u) % 8u));
    bad |= (*p).m1_0 != (_Bool)(abi_mix(s, 7u) & 1u);
    bad |= (*p).m1_1 != (_Bool)(abi_mix(s, 8u) & 1u);
    bad |= (*p).m1_2 != (int)((long long)(abi_mix(s, 9u) % 67108864u) - 33554432);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (short)((long long)(abi_mix(s, 11u) % 65536u) - 32768);
    bad |= (*p).m4 != (_Decimal128)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R73(struct R73 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R73(const struct R73 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R74(struct R74 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (unsigned long long)abi_mix(s, 2u);
    (*p).m3 = (_Decimal128)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m4 = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p).m5[0] = (_Decimal32)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m5[1] = (_Decimal32)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m5[2] = (_Decimal32)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
}
static int check_R74(const struct R74 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (_Decimal32)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (unsigned long long)abi_mix(s, 2u);
    bad |= (*p).m3 != (_Decimal128)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4 != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p).m5[0] != (_Decimal32)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5[1] != (_Decimal32)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5[2] != (_Decimal32)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    return !bad;
}

struct R0 abi_f0(struct R2 a0, ...);
signed char abi_f1(int a0, float a1, struct R7 a2, _Decimal128 a3, union R9 a4, struct R10 a5, struct R12 a6, struct R13 a7, struct R17 a8, unsigned long long a9, struct R19 a10, _Decimal32 a11, ...);
struct R23 abi_f2(int a0, long long a1, int a2, long long a3, int a4, float a5, float a6, struct R25 a7, struct R26 a8, struct R27 a9, struct R28 a10, struct R30 a11, struct R34 a12, struct R35 a13, struct R36 a14, struct R37 a15, _Decimal32 a16);
union R38 abi_f3(struct R39 a0, struct R40 a1, struct R41 a2, _Bool a3);
struct R42 abi_f4(int a0, float a1, long long a2, int a3, int a4, float a5, int a6, int a7, struct R43 a8, struct R46 a9);
union R47 abi_f5(_Decimal32 a0, char * a1, unsigned short a2, struct R49 a3, struct R50 a4, struct R54 a5, double a6, struct R55 a7, struct R56 a8, struct R61 a9);
struct R62 abi_f6(struct R64 a0, struct R66 a1, struct R67 a2, unsigned long long a3, ...);
void abi_f7(struct R68 a0, struct R69 a1, ...);
#endif
