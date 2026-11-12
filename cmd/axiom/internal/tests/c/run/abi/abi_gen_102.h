/* abi_gen_102.h - written by gen_c_abi.py 102 8: the types of the
   tests in abi_gen_102.c and abi_gen_102_callee.c, and the functions that fill
   and check them. */
#ifndef ABI_GEN_102_H
#define ABI_GEN_102_H
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
    double m0;
    double m1;
    double m2[3];
};
struct R1 {
    unsigned char m0;
    float m1[2];
    long long m2;
    unsigned char m3;
};
struct R2 {
    float m0;
    _Bool m1;
};
struct R3 {
    double m0;
    unsigned m1;
    struct R2 m2;
    short m3;
};
struct R4 {
    _Bool m0_0 : 1;
    int m0_1 : 8;
    _Bool m1[3];
    float m2;
    struct R3 m3;
};
struct R5 {
    float m0;
    float m1;
    float m2;
};
struct R6 {
    struct R5 m0;
};
struct R7 {
    unsigned m0_0 : 31;
    _Bool m0_1 : 1;
    unsigned short m1[2];
    short m2;
};
struct R8 {
    float m0;
};
struct R9 {
    long long m0;
};
struct R10 {
    unsigned m0;
};
struct R11 {
    int m0_0 : 23;
    int m0_1 : 30;
    _Bool m0_2 : 1;
    unsigned m1_0 : 23;
    struct R10 m2;
    unsigned m3_0 : 23;
    unsigned m4;
    long long m5;
};
struct R12 {
    int m0_0 : 16;
    unsigned m0_1 : 2;
    _Bool m1;
};
struct R13 {
    unsigned m0;
    short m1;
};
union R14 {
    struct R12 m0;
    struct R13 m1;
    signed char m2[3];
    double m3[3];
    _Bool m4[3];
};
struct R15 {
    union R14 m0;
    short m1;
};
struct R16 {
    unsigned long long m0;
    unsigned long long m1;
};
union R17 {
    unsigned char m0;
    unsigned long long m1;
    _Bool m2;
    unsigned long long m3;
    int m4;
};
struct R18 {
    unsigned short m0;
};
struct R19 {
    _Alignas(16) struct R16 m0;
    unsigned long long m1;
    union R17 m2;
    long long m3;
    signed char m4;
    struct R18 m5;
};
struct R20 {
    short m0;
    float m1;
    _Bool m2[4];
};
union R21 {
    unsigned m0;
    unsigned short m1;
    long long m2[1];
};
struct R22 {
    unsigned short m0[5];
    _Alignas(32) unsigned char m1;
    signed char m2;
    struct R19 m3;
    double m4;
    struct R20 m5;
    char * m6;
    union R21 m7;
};
struct R23 {
    char * m0;
    unsigned m1;
};
struct R24 {
    int m0_0 : 19;
    float m1;
    unsigned char m2;
    unsigned long long m3;
};
struct R25 {
    int m0_0 : 21;
};
struct R26 {
    float m0;
};
struct R27 {
    double m0;
};
union R28 {
    long long m0;
    unsigned short m1;
    struct R27 m2;
    _Bool m3;
    unsigned short m4;
};
struct R29 {
    float m0;
};
struct R30 {
    _Bool m0;
    unsigned m1_0 : 4;
    int m1_1 : 17;
    int m1_2 : 27;
    unsigned m1_3 : 29;
    double m2;
    struct R29 m3;
    unsigned char m4;
    unsigned char m5;
};
struct R31 {
    long long m0;
    unsigned : 3;
    int m1_0 : 11;
    int m1_1 : 1;
    int m1_2 : 25;
    int m2;
};
struct R32 {
    float m0;
    float m1;
    float m2;
    float m3;
};
struct R33 {
    float m0;
    struct R32 m1;
};
struct R34 {
    double m0[2];
    double m1;
};
struct R35 {
    double m0;
};
struct R36 {
    struct R35 m0;
};
struct R37 {
    short m0;
    _Bool m1_0 : 1;
    int m1_1 : 9;
    unsigned short m2;
};
struct R38 {
    signed char m0;
    struct R37 m1;
    double m2[4];
};
struct R39 {
    _Alignas(8) int m0[1];
    unsigned char m1;
    struct R36 m2;
    unsigned m3_0 : 9;
    unsigned m3_1 : 7;
    int m3_2 : 13;
    unsigned char m4;
    double m5[3];
    _Bool m6_0 : 1;
    unsigned m6_1 : 8;
    int : 0;
    unsigned m6_2 : 27;
    unsigned m6_3 : 19;
    struct R38 m7[3];
};
struct R40 {
    float m0;
};
struct R41 {
    float m0;
    struct R40 m1;
    float m2;
};
struct R42 {
    int m0_0 : 21;
};
struct R43 {
    unsigned long long m0;
    double m1[2];
};
struct R44 {
    _Alignas(32) short m0;
    struct R43 m1;
    _Bool m2_0 : 1;
};
struct R45 {
    unsigned m0;
    double m1;
    _Bool m2;
};
struct R46 {
    long long m0;
    _Alignas(64) struct R45 m1;
    unsigned m2_0 : 3;
};
struct R47 {
    char * m0;
    unsigned long long m1;
};
struct R48 {
    unsigned long long m0;
    int m1_0 : 28;
    int : 0;
    unsigned m1_1 : 8;
    unsigned m1_2 : 7;
};
struct R49 {
    char * m0;
    struct R48 m1;
    unsigned char m2;
    _Alignas(8) long long m3;
    unsigned m4_0 : 14;
    _Bool m4_1 : 1;
    int m4_2 : 17;
    int : 0;
    int m4_3 : 28;
};
union R50 {
    unsigned long long m0[1];
    char * m1;
    struct R49 m2;
    _Alignas(8) char * m3[3];
};
struct R51 {
    unsigned char m0[3];
};
struct R52 {
    _Alignas(64) short m0[3];
    unsigned m1;
    double m2;
    long long m3[2];
    unsigned m4_0 : 22;
    _Bool m4_1 : 1;
    signed char m5;
    _Bool m6_0 : 1;
    unsigned m6_1 : 10;
    _Bool m6_2 : 1;
    short m7[3];
};
struct R53 {
    double m0[2];
};
struct R54 {
    double m0;
    double m1;
};
struct R55 {
    unsigned m0;
    unsigned short m1[3];
    int m2_0 : 5;
    _Bool m2_1 : 1;
    int m2_2 : 2;
};
struct R56 {
    int m0_0 : 14;
    unsigned m0_1 : 12;
    unsigned m1;
    char * m2[1];
};
union R57 {
    int m0;
    signed char m1[2];
    _Bool m2;
};
struct R58 {
    unsigned long long m0;
};
struct R59 {
    int m0_0 : 22;
    int : 3;
    unsigned m0_1 : 16;
    unsigned m0_2 : 10;
};
struct R60 {
    short m0;
    double m1;
};
struct R61 {
    float m0;
    unsigned m1;
    struct R60 m2;
};
union R62 {
    signed char m0[5];
    signed char m1[5];
    struct R61 m2;
};
struct R63 {
    int : 3;
    int m0_0 : 9;
};
struct R64 {
    char * m0;
    int m1;
};
struct R65 {
    _Alignas(8) _Bool m0;
    _Alignas(8) long long m1;
};

static void fill_R0(struct R0 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2[1] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m2[2] = (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R0(const struct R0 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[1] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[2] != (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R1(struct R1 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (long long)abi_mix(s, 3u);
    (*p).m3 = (unsigned char)(abi_mix(s, 4u) % 256u);
}
static int check_R1(const struct R1 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (long long)abi_mix(s, 3u);
    bad |= (*p).m3 != (unsigned char)(abi_mix(s, 4u) % 256u);
    return !bad;
}
static void fill_R2(struct R2 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (_Bool)(abi_mix(s, 1u) & 1u);
}
static int check_R2(const struct R2 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (_Bool)(abi_mix(s, 1u) & 1u);
    return !bad;
}
static void fill_R3(struct R3 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (unsigned)(abi_mix(s, 1u) % 4294967296u);
    (*p).m2.m0 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m1 = (_Bool)(abi_mix(s, 3u) & 1u);
    (*p).m3 = (short)((long long)(abi_mix(s, 4u) % 65536u) - 32768);
}
static int check_R3(const struct R3 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (unsigned)(abi_mix(s, 1u) % 4294967296u);
    bad |= (*p).m2.m0 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m1 != (_Bool)(abi_mix(s, 3u) & 1u);
    bad |= (*p).m3 != (short)((long long)(abi_mix(s, 4u) % 65536u) - 32768);
    return !bad;
}
static void fill_R4(struct R4 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m0_1 = (int)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m1[0] = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m1[1] = (_Bool)(abi_mix(s, 3u) & 1u);
    (*p).m1[2] = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0 = (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m1 = (unsigned)(abi_mix(s, 7u) % 4294967296u);
    (*p).m3.m2.m0 = (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m2.m1 = (_Bool)(abi_mix(s, 9u) & 1u);
    (*p).m3.m3 = (short)((long long)(abi_mix(s, 10u) % 65536u) - 32768);
}
static int check_R4(const struct R4 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m0_1 != (int)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m1[0] != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m1[1] != (_Bool)(abi_mix(s, 3u) & 1u);
    bad |= (*p).m1[2] != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0 != (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m1 != (unsigned)(abi_mix(s, 7u) % 4294967296u);
    bad |= (*p).m3.m2.m0 != (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m2.m1 != (_Bool)(abi_mix(s, 9u) & 1u);
    bad |= (*p).m3.m3 != (short)((long long)(abi_mix(s, 10u) % 65536u) - 32768);
    return !bad;
}
static void fill_R5(struct R5 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R5(const struct R5 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R6(struct R6 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R6(const struct R6 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R7(struct R7 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 2147483648u);
    (*p).m0_1 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m1[0] = (unsigned short)(abi_mix(s, 2u) % 65536u);
    (*p).m1[1] = (unsigned short)(abi_mix(s, 3u) % 65536u);
    (*p).m2 = (short)((long long)(abi_mix(s, 4u) % 65536u) - 32768);
}
static int check_R7(const struct R7 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 2147483648u);
    bad |= (*p).m0_1 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m1[0] != (unsigned short)(abi_mix(s, 2u) % 65536u);
    bad |= (*p).m1[1] != (unsigned short)(abi_mix(s, 3u) % 65536u);
    bad |= (*p).m2 != (short)((long long)(abi_mix(s, 4u) % 65536u) - 32768);
    return !bad;
}
static void fill_R8(struct R8 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R8(const struct R8 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R9(struct R9 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (long long)abi_mix(s, 0u);
}
static int check_R9(const struct R9 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (long long)abi_mix(s, 0u);
    return !bad;
}
static void fill_R10(struct R10 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
}
static int check_R10(const struct R10 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    return !bad;
}
static void fill_R11(struct R11 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 8388608u) - 4194304);
    (*p).m0_1 = (int)((long long)(abi_mix(s, 1u) % 1073741824u) - 536870912);
    (*p).m0_2 = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m1_0 = (unsigned)(abi_mix(s, 3u) % 8388608u);
    (*p).m2.m0 = (unsigned)(abi_mix(s, 4u) % 4294967296u);
    (*p).m3_0 = (unsigned)(abi_mix(s, 5u) % 8388608u);
    (*p).m4 = (unsigned)(abi_mix(s, 6u) % 4294967296u);
    (*p).m5 = (long long)abi_mix(s, 7u);
}
static int check_R11(const struct R11 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 8388608u) - 4194304);
    bad |= (*p).m0_1 != (int)((long long)(abi_mix(s, 1u) % 1073741824u) - 536870912);
    bad |= (*p).m0_2 != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 3u) % 8388608u);
    bad |= (*p).m2.m0 != (unsigned)(abi_mix(s, 4u) % 4294967296u);
    bad |= (*p).m3_0 != (unsigned)(abi_mix(s, 5u) % 8388608u);
    bad |= (*p).m4 != (unsigned)(abi_mix(s, 6u) % 4294967296u);
    bad |= (*p).m5 != (long long)abi_mix(s, 7u);
    return !bad;
}
static void fill_R12(struct R12 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 4u);
    (*p).m1 = (_Bool)(abi_mix(s, 2u) & 1u);
}
static int check_R12(const struct R12 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 4u);
    bad |= (*p).m1 != (_Bool)(abi_mix(s, 2u) & 1u);
    return !bad;
}
static void fill_R13(struct R13 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
    (*p).m1 = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
}
static int check_R13(const struct R13 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    bad |= (*p).m1 != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    return !bad;
}
static void fill_R14(union R14 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0_0 = (int)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m0.m0_1 = (unsigned)(abi_mix(s, 1u) % 4u);
    (*p).m0.m1 = (_Bool)(abi_mix(s, 2u) & 1u);
}
static int check_R14(const union R14 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0_0 != (int)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m0.m0_1 != (unsigned)(abi_mix(s, 1u) % 4u);
    bad |= (*p).m0.m1 != (_Bool)(abi_mix(s, 2u) & 1u);
    return !bad;
}
static void fill_R15(struct R15 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0.m0_0 = (int)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m0.m0.m0_1 = (unsigned)(abi_mix(s, 1u) % 4u);
    (*p).m0.m0.m1 = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m1 = (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
}
static int check_R15(const struct R15 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0.m0_0 != (int)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m0.m0.m0_1 != (unsigned)(abi_mix(s, 1u) % 4u);
    bad |= (*p).m0.m0.m1 != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m1 != (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
    return !bad;
}
static void fill_R16(struct R16 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
    (*p).m1 = (unsigned long long)abi_mix(s, 1u);
}
static int check_R16(const struct R16 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m1 != (unsigned long long)abi_mix(s, 1u);
    return !bad;
}
static void fill_R17(union R17 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
}
static int check_R17(const union R17 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    return !bad;
}
static void fill_R18(struct R18 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
}
static int check_R18(const struct R18 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    return !bad;
}
static void fill_R19(struct R19 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (unsigned long long)abi_mix(s, 0u);
    (*p).m0.m1 = (unsigned long long)abi_mix(s, 1u);
    (*p).m1 = (unsigned long long)abi_mix(s, 2u);
    (*p).m2.m0 = (unsigned char)(abi_mix(s, 3u) % 256u);
    (*p).m3 = (long long)abi_mix(s, 4u);
    (*p).m4 = (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    (*p).m5.m0 = (unsigned short)(abi_mix(s, 6u) % 65536u);
}
static int check_R19(const struct R19 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m0.m1 != (unsigned long long)abi_mix(s, 1u);
    bad |= (*p).m1 != (unsigned long long)abi_mix(s, 2u);
    bad |= (*p).m2.m0 != (unsigned char)(abi_mix(s, 3u) % 256u);
    bad |= (*p).m3 != (long long)abi_mix(s, 4u);
    bad |= (*p).m4 != (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    bad |= (*p).m5.m0 != (unsigned short)(abi_mix(s, 6u) % 65536u);
    return !bad;
}
static void fill_R20(struct R20 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m2[1] = (_Bool)(abi_mix(s, 3u) & 1u);
    (*p).m2[2] = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m2[3] = (_Bool)(abi_mix(s, 5u) & 1u);
}
static int check_R20(const struct R20 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m2[1] != (_Bool)(abi_mix(s, 3u) & 1u);
    bad |= (*p).m2[2] != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m2[3] != (_Bool)(abi_mix(s, 5u) & 1u);
    return !bad;
}
static void fill_R21(union R21 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
}
static int check_R21(const union R21 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    return !bad;
}
static void fill_R22(struct R22 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m0[1] = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m0[2] = (unsigned short)(abi_mix(s, 2u) % 65536u);
    (*p).m0[3] = (unsigned short)(abi_mix(s, 3u) % 65536u);
    (*p).m0[4] = (unsigned short)(abi_mix(s, 4u) % 65536u);
    (*p).m1 = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m2 = (signed char)((long long)(abi_mix(s, 6u) % 256u) - 128);
    (*p).m3.m0.m0 = (unsigned long long)abi_mix(s, 7u);
    (*p).m3.m0.m1 = (unsigned long long)abi_mix(s, 8u);
    (*p).m3.m1 = (unsigned long long)abi_mix(s, 9u);
    (*p).m3.m2.m0 = (unsigned char)(abi_mix(s, 10u) % 256u);
    (*p).m3.m3 = (long long)abi_mix(s, 11u);
    (*p).m3.m4 = (signed char)((long long)(abi_mix(s, 12u) % 256u) - 128);
    (*p).m3.m5.m0 = (unsigned short)(abi_mix(s, 13u) % 65536u);
    (*p).m4 = (double)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    (*p).m5.m0 = (short)((long long)(abi_mix(s, 15u) % 65536u) - 32768);
    (*p).m5.m1 = (float)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
    (*p).m5.m2[0] = (_Bool)(abi_mix(s, 17u) & 1u);
    (*p).m5.m2[1] = (_Bool)(abi_mix(s, 18u) & 1u);
    (*p).m5.m2[2] = (_Bool)(abi_mix(s, 19u) & 1u);
    (*p).m5.m2[3] = (_Bool)(abi_mix(s, 20u) & 1u);
    (*p).m6 = (abi_anchor + (abi_mix(s, 21u) % 8u));
    (*p).m7.m0 = (unsigned)(abi_mix(s, 22u) % 4294967296u);
}
static int check_R22(const struct R22 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m0[1] != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m0[2] != (unsigned short)(abi_mix(s, 2u) % 65536u);
    bad |= (*p).m0[3] != (unsigned short)(abi_mix(s, 3u) % 65536u);
    bad |= (*p).m0[4] != (unsigned short)(abi_mix(s, 4u) % 65536u);
    bad |= (*p).m1 != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m2 != (signed char)((long long)(abi_mix(s, 6u) % 256u) - 128);
    bad |= (*p).m3.m0.m0 != (unsigned long long)abi_mix(s, 7u);
    bad |= (*p).m3.m0.m1 != (unsigned long long)abi_mix(s, 8u);
    bad |= (*p).m3.m1 != (unsigned long long)abi_mix(s, 9u);
    bad |= (*p).m3.m2.m0 != (unsigned char)(abi_mix(s, 10u) % 256u);
    bad |= (*p).m3.m3 != (long long)abi_mix(s, 11u);
    bad |= (*p).m3.m4 != (signed char)((long long)(abi_mix(s, 12u) % 256u) - 128);
    bad |= (*p).m3.m5.m0 != (unsigned short)(abi_mix(s, 13u) % 65536u);
    bad |= (*p).m4 != (double)((double)(long long)(abi_mix(s, 14u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5.m0 != (short)((long long)(abi_mix(s, 15u) % 65536u) - 32768);
    bad |= (*p).m5.m1 != (float)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5.m2[0] != (_Bool)(abi_mix(s, 17u) & 1u);
    bad |= (*p).m5.m2[1] != (_Bool)(abi_mix(s, 18u) & 1u);
    bad |= (*p).m5.m2[2] != (_Bool)(abi_mix(s, 19u) & 1u);
    bad |= (*p).m5.m2[3] != (_Bool)(abi_mix(s, 20u) & 1u);
    bad |= (*p).m6 != (abi_anchor + (abi_mix(s, 21u) % 8u));
    bad |= (*p).m7.m0 != (unsigned)(abi_mix(s, 22u) % 4294967296u);
    return !bad;
}
static void fill_R23(struct R23 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
    (*p).m1 = (unsigned)(abi_mix(s, 1u) % 4294967296u);
}
static int check_R23(const struct R23 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    bad |= (*p).m1 != (unsigned)(abi_mix(s, 1u) % 4294967296u);
    return !bad;
}
static void fill_R24(struct R24 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 524288u) - 262144);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p).m3 = (unsigned long long)abi_mix(s, 3u);
}
static int check_R24(const struct R24 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 524288u) - 262144);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m3 != (unsigned long long)abi_mix(s, 3u);
    return !bad;
}
static void fill_R25(struct R25 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 2097152u) - 1048576);
}
static int check_R25(const struct R25 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 2097152u) - 1048576);
    return !bad;
}
static void fill_R26(struct R26 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R26(const struct R26 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
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
static void fill_R28(union R28 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (long long)abi_mix(s, 0u);
}
static int check_R28(const union R28 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (long long)abi_mix(s, 0u);
    return !bad;
}
static void fill_R29(struct R29 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R29(const struct R29 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R30(struct R30 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m1_0 = (unsigned)(abi_mix(s, 1u) % 16u);
    (*p).m1_1 = (int)((long long)(abi_mix(s, 2u) % 131072u) - 65536);
    (*p).m1_2 = (int)((long long)(abi_mix(s, 3u) % 134217728u) - 67108864);
    (*p).m1_3 = (unsigned)(abi_mix(s, 4u) % 536870912u);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m0 = (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m4 = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m5 = (unsigned char)(abi_mix(s, 8u) % 256u);
}
static int check_R30(const struct R30 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 1u) % 16u);
    bad |= (*p).m1_1 != (int)((long long)(abi_mix(s, 2u) % 131072u) - 65536);
    bad |= (*p).m1_2 != (int)((long long)(abi_mix(s, 3u) % 134217728u) - 67108864);
    bad |= (*p).m1_3 != (unsigned)(abi_mix(s, 4u) % 536870912u);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m0 != (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4 != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m5 != (unsigned char)(abi_mix(s, 8u) % 256u);
    return !bad;
}
static void fill_R31(struct R31 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (long long)abi_mix(s, 0u);
    (*p).m1_0 = (int)((long long)(abi_mix(s, 1u) % 2048u) - 1024);
    (*p).m1_1 = (int)((long long)(abi_mix(s, 2u) % 2u) - 1);
    (*p).m1_2 = (int)((long long)(abi_mix(s, 3u) % 33554432u) - 16777216);
    (*p).m2 = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
}
static int check_R31(const struct R31 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (long long)abi_mix(s, 0u);
    bad |= (*p).m1_0 != (int)((long long)(abi_mix(s, 1u) % 2048u) - 1024);
    bad |= (*p).m1_1 != (int)((long long)(abi_mix(s, 2u) % 2u) - 1);
    bad |= (*p).m1_2 != (int)((long long)(abi_mix(s, 3u) % 33554432u) - 16777216);
    bad |= (*p).m2 != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R32(struct R32 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R32(const struct R32 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R33(struct R33 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2 = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m3 = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R33(const struct R33 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2 != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m3 != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R34(struct R34 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R34(const struct R34 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R35(struct R35 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R35(const struct R35 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R36(struct R36 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R36(const struct R36 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R37(struct R37 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m1_0 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m1_1 = (int)((long long)(abi_mix(s, 2u) % 512u) - 256);
    (*p).m2 = (unsigned short)(abi_mix(s, 3u) % 65536u);
}
static int check_R37(const struct R37 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m1_0 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m1_1 != (int)((long long)(abi_mix(s, 2u) % 512u) - 256);
    bad |= (*p).m2 != (unsigned short)(abi_mix(s, 3u) % 65536u);
    return !bad;
}
static void fill_R38(struct R38 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m1.m0 = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    (*p).m1.m1_0 = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m1.m1_1 = (int)((long long)(abi_mix(s, 3u) % 512u) - 256);
    (*p).m1.m2 = (unsigned short)(abi_mix(s, 4u) % 65536u);
    (*p).m2[0] = (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m2[1] = (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m2[2] = (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m2[3] = (double)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
}
static int check_R38(const struct R38 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m1.m0 != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    bad |= (*p).m1.m1_0 != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m1.m1_1 != (int)((long long)(abi_mix(s, 3u) % 512u) - 256);
    bad |= (*p).m1.m2 != (unsigned short)(abi_mix(s, 4u) % 65536u);
    bad |= (*p).m2[0] != (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[1] != (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[2] != (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[3] != (double)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R39(struct R39 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p).m1 = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m2.m0.m0 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m3_0 = (unsigned)(abi_mix(s, 3u) % 512u);
    (*p).m3_1 = (unsigned)(abi_mix(s, 4u) % 128u);
    (*p).m3_2 = (int)((long long)(abi_mix(s, 5u) % 8192u) - 4096);
    (*p).m4 = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m5[0] = (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m5[1] = (double)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    (*p).m5[2] = (double)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    (*p).m6_0 = (_Bool)(abi_mix(s, 10u) & 1u);
    (*p).m6_1 = (unsigned)(abi_mix(s, 11u) % 256u);
    (*p).m6_2 = (unsigned)(abi_mix(s, 12u) % 134217728u);
    (*p).m6_3 = (unsigned)(abi_mix(s, 13u) % 524288u);
    (*p).m7[0].m0 = (signed char)((long long)(abi_mix(s, 14u) % 256u) - 128);
    (*p).m7[0].m1.m0 = (short)((long long)(abi_mix(s, 15u) % 65536u) - 32768);
    (*p).m7[0].m1.m1_0 = (_Bool)(abi_mix(s, 16u) & 1u);
    (*p).m7[0].m1.m1_1 = (int)((long long)(abi_mix(s, 17u) % 512u) - 256);
    (*p).m7[0].m1.m2 = (unsigned short)(abi_mix(s, 18u) % 65536u);
    (*p).m7[0].m2[0] = (double)((double)(long long)(abi_mix(s, 19u) % 4001u) / 8.0 - 250.0);
    (*p).m7[0].m2[1] = (double)((double)(long long)(abi_mix(s, 20u) % 4001u) / 8.0 - 250.0);
    (*p).m7[0].m2[2] = (double)((double)(long long)(abi_mix(s, 21u) % 4001u) / 8.0 - 250.0);
    (*p).m7[0].m2[3] = (double)((double)(long long)(abi_mix(s, 22u) % 4001u) / 8.0 - 250.0);
    (*p).m7[1].m0 = (signed char)((long long)(abi_mix(s, 23u) % 256u) - 128);
    (*p).m7[1].m1.m0 = (short)((long long)(abi_mix(s, 24u) % 65536u) - 32768);
    (*p).m7[1].m1.m1_0 = (_Bool)(abi_mix(s, 25u) & 1u);
    (*p).m7[1].m1.m1_1 = (int)((long long)(abi_mix(s, 26u) % 512u) - 256);
    (*p).m7[1].m1.m2 = (unsigned short)(abi_mix(s, 27u) % 65536u);
    (*p).m7[1].m2[0] = (double)((double)(long long)(abi_mix(s, 28u) % 4001u) / 8.0 - 250.0);
    (*p).m7[1].m2[1] = (double)((double)(long long)(abi_mix(s, 29u) % 4001u) / 8.0 - 250.0);
    (*p).m7[1].m2[2] = (double)((double)(long long)(abi_mix(s, 30u) % 4001u) / 8.0 - 250.0);
    (*p).m7[1].m2[3] = (double)((double)(long long)(abi_mix(s, 31u) % 4001u) / 8.0 - 250.0);
    (*p).m7[2].m0 = (signed char)((long long)(abi_mix(s, 32u) % 256u) - 128);
    (*p).m7[2].m1.m0 = (short)((long long)(abi_mix(s, 33u) % 65536u) - 32768);
    (*p).m7[2].m1.m1_0 = (_Bool)(abi_mix(s, 34u) & 1u);
    (*p).m7[2].m1.m1_1 = (int)((long long)(abi_mix(s, 35u) % 512u) - 256);
    (*p).m7[2].m1.m2 = (unsigned short)(abi_mix(s, 36u) % 65536u);
    (*p).m7[2].m2[0] = (double)((double)(long long)(abi_mix(s, 37u) % 4001u) / 8.0 - 250.0);
    (*p).m7[2].m2[1] = (double)((double)(long long)(abi_mix(s, 38u) % 4001u) / 8.0 - 250.0);
    (*p).m7[2].m2[2] = (double)((double)(long long)(abi_mix(s, 39u) % 4001u) / 8.0 - 250.0);
    (*p).m7[2].m2[3] = (double)((double)(long long)(abi_mix(s, 40u) % 4001u) / 8.0 - 250.0);
}
static int check_R39(const struct R39 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p).m1 != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m2.m0.m0 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3_0 != (unsigned)(abi_mix(s, 3u) % 512u);
    bad |= (*p).m3_1 != (unsigned)(abi_mix(s, 4u) % 128u);
    bad |= (*p).m3_2 != (int)((long long)(abi_mix(s, 5u) % 8192u) - 4096);
    bad |= (*p).m4 != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m5[0] != (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5[1] != (double)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5[2] != (double)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m6_0 != (_Bool)(abi_mix(s, 10u) & 1u);
    bad |= (*p).m6_1 != (unsigned)(abi_mix(s, 11u) % 256u);
    bad |= (*p).m6_2 != (unsigned)(abi_mix(s, 12u) % 134217728u);
    bad |= (*p).m6_3 != (unsigned)(abi_mix(s, 13u) % 524288u);
    bad |= (*p).m7[0].m0 != (signed char)((long long)(abi_mix(s, 14u) % 256u) - 128);
    bad |= (*p).m7[0].m1.m0 != (short)((long long)(abi_mix(s, 15u) % 65536u) - 32768);
    bad |= (*p).m7[0].m1.m1_0 != (_Bool)(abi_mix(s, 16u) & 1u);
    bad |= (*p).m7[0].m1.m1_1 != (int)((long long)(abi_mix(s, 17u) % 512u) - 256);
    bad |= (*p).m7[0].m1.m2 != (unsigned short)(abi_mix(s, 18u) % 65536u);
    bad |= (*p).m7[0].m2[0] != (double)((double)(long long)(abi_mix(s, 19u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m7[0].m2[1] != (double)((double)(long long)(abi_mix(s, 20u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m7[0].m2[2] != (double)((double)(long long)(abi_mix(s, 21u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m7[0].m2[3] != (double)((double)(long long)(abi_mix(s, 22u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m7[1].m0 != (signed char)((long long)(abi_mix(s, 23u) % 256u) - 128);
    bad |= (*p).m7[1].m1.m0 != (short)((long long)(abi_mix(s, 24u) % 65536u) - 32768);
    bad |= (*p).m7[1].m1.m1_0 != (_Bool)(abi_mix(s, 25u) & 1u);
    bad |= (*p).m7[1].m1.m1_1 != (int)((long long)(abi_mix(s, 26u) % 512u) - 256);
    bad |= (*p).m7[1].m1.m2 != (unsigned short)(abi_mix(s, 27u) % 65536u);
    bad |= (*p).m7[1].m2[0] != (double)((double)(long long)(abi_mix(s, 28u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m7[1].m2[1] != (double)((double)(long long)(abi_mix(s, 29u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m7[1].m2[2] != (double)((double)(long long)(abi_mix(s, 30u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m7[1].m2[3] != (double)((double)(long long)(abi_mix(s, 31u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m7[2].m0 != (signed char)((long long)(abi_mix(s, 32u) % 256u) - 128);
    bad |= (*p).m7[2].m1.m0 != (short)((long long)(abi_mix(s, 33u) % 65536u) - 32768);
    bad |= (*p).m7[2].m1.m1_0 != (_Bool)(abi_mix(s, 34u) & 1u);
    bad |= (*p).m7[2].m1.m1_1 != (int)((long long)(abi_mix(s, 35u) % 512u) - 256);
    bad |= (*p).m7[2].m1.m2 != (unsigned short)(abi_mix(s, 36u) % 65536u);
    bad |= (*p).m7[2].m2[0] != (double)((double)(long long)(abi_mix(s, 37u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m7[2].m2[1] != (double)((double)(long long)(abi_mix(s, 38u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m7[2].m2[2] != (double)((double)(long long)(abi_mix(s, 39u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m7[2].m2[3] != (double)((double)(long long)(abi_mix(s, 40u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R40(struct R40 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R40(const struct R40 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R41(struct R41 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R41(const struct R41 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R42(struct R42 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 2097152u) - 1048576);
}
static int check_R42(const struct R42 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 2097152u) - 1048576);
    return !bad;
}
static void fill_R43(struct R43 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
    (*p).m1[0] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R43(const struct R43 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m1[0] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R44(struct R44 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m1.m0 = (unsigned long long)abi_mix(s, 1u);
    (*p).m1.m1[0] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1[1] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m2_0 = (_Bool)(abi_mix(s, 4u) & 1u);
}
static int check_R44(const struct R44 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m1.m0 != (unsigned long long)abi_mix(s, 1u);
    bad |= (*p).m1.m1[0] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1[1] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2_0 != (_Bool)(abi_mix(s, 4u) & 1u);
    return !bad;
}
static void fill_R45(struct R45 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (_Bool)(abi_mix(s, 2u) & 1u);
}
static int check_R45(const struct R45 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (_Bool)(abi_mix(s, 2u) & 1u);
    return !bad;
}
static void fill_R46(struct R46 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (long long)abi_mix(s, 0u);
    (*p).m1.m0 = (unsigned)(abi_mix(s, 1u) % 4294967296u);
    (*p).m1.m1 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2 = (_Bool)(abi_mix(s, 3u) & 1u);
    (*p).m2_0 = (unsigned)(abi_mix(s, 4u) % 8u);
}
static int check_R46(const struct R46 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (long long)abi_mix(s, 0u);
    bad |= (*p).m1.m0 != (unsigned)(abi_mix(s, 1u) % 4294967296u);
    bad |= (*p).m1.m1 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2 != (_Bool)(abi_mix(s, 3u) & 1u);
    bad |= (*p).m2_0 != (unsigned)(abi_mix(s, 4u) % 8u);
    return !bad;
}
static void fill_R47(struct R47 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
    (*p).m1 = (unsigned long long)abi_mix(s, 1u);
}
static int check_R47(const struct R47 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    bad |= (*p).m1 != (unsigned long long)abi_mix(s, 1u);
    return !bad;
}
static void fill_R48(struct R48 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
    (*p).m1_0 = (int)((long long)(abi_mix(s, 1u) % 268435456u) - 134217728);
    (*p).m1_1 = (unsigned)(abi_mix(s, 2u) % 256u);
    (*p).m1_2 = (unsigned)(abi_mix(s, 3u) % 128u);
}
static int check_R48(const struct R48 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m1_0 != (int)((long long)(abi_mix(s, 1u) % 268435456u) - 134217728);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m1_2 != (unsigned)(abi_mix(s, 3u) % 128u);
    return !bad;
}
static void fill_R49(struct R49 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
    (*p).m1.m0 = (unsigned long long)abi_mix(s, 1u);
    (*p).m1.m1_0 = (int)((long long)(abi_mix(s, 2u) % 268435456u) - 134217728);
    (*p).m1.m1_1 = (unsigned)(abi_mix(s, 3u) % 256u);
    (*p).m1.m1_2 = (unsigned)(abi_mix(s, 4u) % 128u);
    (*p).m2 = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m3 = (long long)abi_mix(s, 6u);
    (*p).m4_0 = (unsigned)(abi_mix(s, 7u) % 16384u);
    (*p).m4_1 = (_Bool)(abi_mix(s, 8u) & 1u);
    (*p).m4_2 = (int)((long long)(abi_mix(s, 9u) % 131072u) - 65536);
    (*p).m4_3 = (int)((long long)(abi_mix(s, 10u) % 268435456u) - 134217728);
}
static int check_R49(const struct R49 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    bad |= (*p).m1.m0 != (unsigned long long)abi_mix(s, 1u);
    bad |= (*p).m1.m1_0 != (int)((long long)(abi_mix(s, 2u) % 268435456u) - 134217728);
    bad |= (*p).m1.m1_1 != (unsigned)(abi_mix(s, 3u) % 256u);
    bad |= (*p).m1.m1_2 != (unsigned)(abi_mix(s, 4u) % 128u);
    bad |= (*p).m2 != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m3 != (long long)abi_mix(s, 6u);
    bad |= (*p).m4_0 != (unsigned)(abi_mix(s, 7u) % 16384u);
    bad |= (*p).m4_1 != (_Bool)(abi_mix(s, 8u) & 1u);
    bad |= (*p).m4_2 != (int)((long long)(abi_mix(s, 9u) % 131072u) - 65536);
    bad |= (*p).m4_3 != (int)((long long)(abi_mix(s, 10u) % 268435456u) - 134217728);
    return !bad;
}
static void fill_R50(union R50 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned long long)abi_mix(s, 0u);
}
static int check_R50(const union R50 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned long long)abi_mix(s, 0u);
    return !bad;
}
static void fill_R51(struct R51 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m0[1] = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m0[2] = (unsigned char)(abi_mix(s, 2u) % 256u);
}
static int check_R51(const struct R51 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m0[1] != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m0[2] != (unsigned char)(abi_mix(s, 2u) % 256u);
    return !bad;
}
static void fill_R52(struct R52 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m0[1] = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    (*p).m0[2] = (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    (*p).m1 = (unsigned)(abi_mix(s, 3u) % 4294967296u);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m3[0] = (long long)abi_mix(s, 5u);
    (*p).m3[1] = (long long)abi_mix(s, 6u);
    (*p).m4_0 = (unsigned)(abi_mix(s, 7u) % 4194304u);
    (*p).m4_1 = (_Bool)(abi_mix(s, 8u) & 1u);
    (*p).m5 = (signed char)((long long)(abi_mix(s, 9u) % 256u) - 128);
    (*p).m6_0 = (_Bool)(abi_mix(s, 10u) & 1u);
    (*p).m6_1 = (unsigned)(abi_mix(s, 11u) % 1024u);
    (*p).m6_2 = (_Bool)(abi_mix(s, 12u) & 1u);
    (*p).m7[0] = (short)((long long)(abi_mix(s, 13u) % 65536u) - 32768);
    (*p).m7[1] = (short)((long long)(abi_mix(s, 14u) % 65536u) - 32768);
    (*p).m7[2] = (short)((long long)(abi_mix(s, 15u) % 65536u) - 32768);
}
static int check_R52(const struct R52 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m0[1] != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    bad |= (*p).m0[2] != (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    bad |= (*p).m1 != (unsigned)(abi_mix(s, 3u) % 4294967296u);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[0] != (long long)abi_mix(s, 5u);
    bad |= (*p).m3[1] != (long long)abi_mix(s, 6u);
    bad |= (*p).m4_0 != (unsigned)(abi_mix(s, 7u) % 4194304u);
    bad |= (*p).m4_1 != (_Bool)(abi_mix(s, 8u) & 1u);
    bad |= (*p).m5 != (signed char)((long long)(abi_mix(s, 9u) % 256u) - 128);
    bad |= (*p).m6_0 != (_Bool)(abi_mix(s, 10u) & 1u);
    bad |= (*p).m6_1 != (unsigned)(abi_mix(s, 11u) % 1024u);
    bad |= (*p).m6_2 != (_Bool)(abi_mix(s, 12u) & 1u);
    bad |= (*p).m7[0] != (short)((long long)(abi_mix(s, 13u) % 65536u) - 32768);
    bad |= (*p).m7[1] != (short)((long long)(abi_mix(s, 14u) % 65536u) - 32768);
    bad |= (*p).m7[2] != (short)((long long)(abi_mix(s, 15u) % 65536u) - 32768);
    return !bad;
}
static void fill_R53(struct R53 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R53(const struct R53 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R54(struct R54 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R54(const struct R54 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R55(struct R55 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
    (*p).m1[0] = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m1[1] = (unsigned short)(abi_mix(s, 2u) % 65536u);
    (*p).m1[2] = (unsigned short)(abi_mix(s, 3u) % 65536u);
    (*p).m2_0 = (int)((long long)(abi_mix(s, 4u) % 32u) - 16);
    (*p).m2_1 = (_Bool)(abi_mix(s, 5u) & 1u);
    (*p).m2_2 = (int)((long long)(abi_mix(s, 6u) % 4u) - 2);
}
static int check_R55(const struct R55 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    bad |= (*p).m1[0] != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m1[1] != (unsigned short)(abi_mix(s, 2u) % 65536u);
    bad |= (*p).m1[2] != (unsigned short)(abi_mix(s, 3u) % 65536u);
    bad |= (*p).m2_0 != (int)((long long)(abi_mix(s, 4u) % 32u) - 16);
    bad |= (*p).m2_1 != (_Bool)(abi_mix(s, 5u) & 1u);
    bad |= (*p).m2_2 != (int)((long long)(abi_mix(s, 6u) % 4u) - 2);
    return !bad;
}
static void fill_R56(struct R56 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 16384u) - 8192);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 4096u);
    (*p).m1 = (unsigned)(abi_mix(s, 2u) % 4294967296u);
    (*p).m2[0] = (abi_anchor + (abi_mix(s, 3u) % 8u));
}
static int check_R56(const struct R56 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 16384u) - 8192);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 4096u);
    bad |= (*p).m1 != (unsigned)(abi_mix(s, 2u) % 4294967296u);
    bad |= (*p).m2[0] != (abi_anchor + (abi_mix(s, 3u) % 8u));
    return !bad;
}
static void fill_R57(union R57 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
}
static int check_R57(const union R57 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R58(struct R58 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
}
static int check_R58(const struct R58 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    return !bad;
}
static void fill_R59(struct R59 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 4194304u) - 2097152);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 65536u);
    (*p).m0_2 = (unsigned)(abi_mix(s, 2u) % 1024u);
}
static int check_R59(const struct R59 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 4194304u) - 2097152);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m0_2 != (unsigned)(abi_mix(s, 2u) % 1024u);
    return !bad;
}
static void fill_R60(struct R60 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R60(const struct R60 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R61(struct R61 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (unsigned)(abi_mix(s, 1u) % 4294967296u);
    (*p).m2.m0 = (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    (*p).m2.m1 = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R61(const struct R61 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (unsigned)(abi_mix(s, 1u) % 4294967296u);
    bad |= (*p).m2.m0 != (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    bad |= (*p).m2.m1 != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R62(union R62 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0[1] = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m0[2] = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m0[3] = (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    (*p).m0[4] = (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
}
static int check_R62(const union R62 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0[1] != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m0[2] != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m0[3] != (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    bad |= (*p).m0[4] != (signed char)((long long)(abi_mix(s, 4u) % 256u) - 128);
    return !bad;
}
static void fill_R63(struct R63 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 512u) - 256);
}
static int check_R63(const struct R63 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 512u) - 256);
    return !bad;
}
static void fill_R64(struct R64 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
    (*p).m1 = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
}
static int check_R64(const struct R64 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    bad |= (*p).m1 != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R65(struct R65 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m1 = (long long)abi_mix(s, 1u);
}
static int check_R65(const struct R65 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m1 != (long long)abi_mix(s, 1u);
    return !bad;
}

void abi_f0(struct R0 a0, struct R1 a1, ...);
void abi_f1(double a0, int a1, int a2, double a3, int a4, int a5, struct R6 a6, unsigned long long a7, unsigned long long a8, struct R7 a9, struct R8 a10, struct R9 a11, struct R11 a12);
float abi_f2(void);
struct R15 abi_f3(float a0, float a1, struct R22 a2, _Bool a3, struct R23 a4, long long a5, double a6);
struct R24 abi_f4(struct R25 a0);
struct R26 abi_f5(union R28 a0, struct R30 a1, ...);
struct R39 abi_f6(char * a0, struct R41 a1, _Bool a2, char * a3, struct R42 a4, struct R44 a5, struct R46 a6, struct R47 a7, ...);
long long abi_f7(double a0, unsigned a1, unsigned long long a2, struct R56 a3, int a4, double a5, union R57 a6, struct R58 a7, struct R59 a8, ...);
#endif
