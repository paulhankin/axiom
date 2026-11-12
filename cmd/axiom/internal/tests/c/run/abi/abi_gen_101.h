/* abi_gen_101.h - written by gen_c_abi.py 101 8: the types of the
   tests in abi_gen_101.c and abi_gen_101_callee.c, and the functions that fill
   and check them. */
#ifndef ABI_GEN_101_H
#define ABI_GEN_101_H
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
    unsigned short m0;
    unsigned short m1;
    unsigned short m2[1];
};
struct R1 {
    unsigned short m0;
};
struct R2 {
    unsigned long long m0[5];
    int m1;
    int m2;
};
struct R3 {
    long long m0;
    short m1;
    unsigned short m2;
};
struct R4 {
    float m0;
    float m1;
};
struct R5 {
    _Bool m0;
    unsigned long long m1;
    int : 3;
    int m2_0 : 12;
    unsigned m2_1 : 32;
    int m2_2 : 30;
    unsigned m2_3 : 29;
};
struct R6 {
    unsigned m0_0 : 32;
    unsigned m0_1 : 9;
    signed char m1;
    int m2;
    int m3_0 : 5;
    unsigned m3_1 : 27;
    unsigned m3_2 : 21;
    char * m4;
};
struct R7 {
    int m0_0 : 10;
    unsigned : 0;
    unsigned m0_1 : 26;
    unsigned m0_2 : 14;
    int m0_3 : 28;
};
struct R8 {
    struct R7 m0;
    unsigned m1_0 : 24;
};
struct R9 {
    unsigned char m0;
    struct R8 m1;
};
struct R10 {
    unsigned long long m0;
    unsigned long long m1[3];
};
struct R11 {
    unsigned char m0;
    unsigned m1_0 : 27;
    unsigned m2;
};
union R12 {
    unsigned short m0;
    float m1;
    struct R11 m2;
    unsigned short m3;
    int m4;
};
union R13 {
    unsigned char m0;
    union R12 m1[2];
    short m2;
};
struct R14 {
    unsigned short m0;
    _Bool m1_0 : 1;
    unsigned char m2;
};
struct R15 {
    _Bool m0_0 : 1;
    short m1[1];
    short m2;
    long long m3;
};
struct R16 {
    int m0_0 : 24;
    int : 3;
    _Bool m0_1 : 1;
    signed char m1[2];
};
struct R17 {
    struct R16 m0;
    float m1;
    long long m2;
};
struct R18 {
    unsigned m0_0 : 17;
    int m0_1 : 15;
    unsigned m0_2 : 7;
    unsigned m0_3 : 13;
    unsigned m1_0 : 20;
    signed char m2;
    float m3[4];
};
struct R19 {
    short m0;
};
struct R20 {
    short m0;
};
struct R21 {
    float m0;
};
struct R22 {
    struct R21 m0;
    float m1;
};
struct R23 {
    unsigned short m0;
    float m1;
    double m2;
};
struct R24 {
    struct R23 m0;
};
struct R25 {
    short m0;
};
union R26 {
    struct R25 m0;
    float m1;
    int m2[5];
};
struct R27 {
    union R26 m0;
    float m1;
    int m2_0 : 8;
    _Bool m2_1 : 1;
    _Bool m2_2 : 1;
    unsigned m2_3 : 5;
};
struct R28 {
    signed char m0;
    int m1;
};
union R29 {
    struct R28 m0;
};
struct R30 {
    unsigned short m0;
    int m1[5];
};
struct R31 {
    union R29 m0;
    _Alignas(64) unsigned m1;
    short m2;
    struct R30 m3;
    _Bool m4;
    int m5_0 : 16;
    unsigned m5_1 : 31;
};
struct R32 {
    float m0[5];
    _Alignas(8) long long m1;
    unsigned m2;
};
struct R33 {
    unsigned short m0[5];
    int m1_0 : 12;
    unsigned m1_1 : 11;
    int m1_2 : 12;
    unsigned char m2;
    float m3[2];
    unsigned m4_0 : 2;
};
struct R34 {
    _Alignas(16) unsigned short m0;
    struct R33 m1;
};
struct R35 {
    long long m0;
    _Alignas(8) char * m1[1];
};
struct R36 {
    unsigned char m0;
    unsigned short m1;
    float m2;
    unsigned char m3;
};
struct R37 {
    long long m0;
    short m1;
    float m2;
    int m3;
    unsigned char m4;
    double m5;
    _Bool m6;
    unsigned char m7;
};
struct R38 {
    struct R37 m0;
    int m1;
    unsigned short m2;
};
struct R39 {
    unsigned long long m0;
    double m1[2];
    struct R35 m2;
    struct R36 m3;
    signed char m4;
    signed char m5;
    struct R38 m6;
    _Alignas(64) double m7;
};
struct R40 {
    _Bool m0[2];
};
union R41 {
    unsigned char m0;
    unsigned short m1;
};
struct R42 {
    float m0;
    float m1;
};
struct R43 {
    struct R42 m0;
};
struct R44 {
    unsigned m0_0 : 4;
    unsigned m0_1 : 27;
    int m0_2 : 30;
};
struct R45 {
    char * m0;
    unsigned short m1;
};
struct R46 {
    int m0[4];
    struct R45 m1;
    unsigned char m2;
    int m3;
};
union R47 {
    signed char m0;
    int m1;
    struct R46 m2;
};
struct R48 {
    unsigned short m0;
};
struct R49 {
    double m0[1];
    double m1[1];
    double m2;
};
struct R50 {
    unsigned m0_0 : 7;
    _Bool m0_1 : 1;
    unsigned m1;
    char * m2;
};
struct R51 {
    unsigned m0;
    unsigned short m1;
    _Bool m2_0 : 1;
    _Bool m2_1 : 1;
    unsigned m2_2 : 8;
};
struct R52 {
    long long m0;
    signed char m1;
    _Bool m2;
    double m3[5];
    unsigned char m4[1];
};
union R53 {
    double m0;
    unsigned char m1;
    _Bool m2;
    char * m3[3];
    float m4;
};
struct R54 {
    char * m0;
    char * m1;
};
struct R55 {
    unsigned m0_0 : 24;
    unsigned : 0;
    unsigned m0_1 : 27;
    short m1;
    unsigned short m2;
    float m3;
    signed char m4;
    char * m5[3];
};
struct R56 {
    signed char m0;
    long long m1;
};
struct R57 {
    struct R56 m0;
    long long m1;
};
struct R58 {
    unsigned m0_0 : 10;
    int : 3;
    unsigned m0_1 : 18;
    unsigned : 3;
    _Bool m0_2 : 1;
    _Bool m0_3 : 1;
};
struct R59 {
    float m0;
    float m1;
};

static void fill_R0(struct R0 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m1 = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m2[0] = (unsigned short)(abi_mix(s, 2u) % 65536u);
}
static int check_R0(const struct R0 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m1 != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m2[0] != (unsigned short)(abi_mix(s, 2u) % 65536u);
    return !bad;
}
static void fill_R1(struct R1 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
}
static int check_R1(const struct R1 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    return !bad;
}
static void fill_R2(struct R2 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned long long)abi_mix(s, 0u);
    (*p).m0[1] = (unsigned long long)abi_mix(s, 1u);
    (*p).m0[2] = (unsigned long long)abi_mix(s, 2u);
    (*p).m0[3] = (unsigned long long)abi_mix(s, 3u);
    (*p).m0[4] = (unsigned long long)abi_mix(s, 4u);
    (*p).m1 = (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    (*p).m2 = (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
}
static int check_R2(const struct R2 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m0[1] != (unsigned long long)abi_mix(s, 1u);
    bad |= (*p).m0[2] != (unsigned long long)abi_mix(s, 2u);
    bad |= (*p).m0[3] != (unsigned long long)abi_mix(s, 3u);
    bad |= (*p).m0[4] != (unsigned long long)abi_mix(s, 4u);
    bad |= (*p).m1 != (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    bad |= (*p).m2 != (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R3(struct R3 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (long long)abi_mix(s, 0u);
    (*p).m1 = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    (*p).m2 = (unsigned short)(abi_mix(s, 2u) % 65536u);
}
static int check_R3(const struct R3 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (long long)abi_mix(s, 0u);
    bad |= (*p).m1 != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    bad |= (*p).m2 != (unsigned short)(abi_mix(s, 2u) % 65536u);
    return !bad;
}
static void fill_R4(struct R4 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R4(const struct R4 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R5(struct R5 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m1 = (unsigned long long)abi_mix(s, 1u);
    (*p).m2_0 = (int)((long long)(abi_mix(s, 2u) % 4096u) - 2048);
    (*p).m2_1 = (unsigned)(abi_mix(s, 3u) % 4294967296u);
    (*p).m2_2 = (int)((long long)(abi_mix(s, 4u) % 1073741824u) - 536870912);
    (*p).m2_3 = (unsigned)(abi_mix(s, 5u) % 536870912u);
}
static int check_R5(const struct R5 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m1 != (unsigned long long)abi_mix(s, 1u);
    bad |= (*p).m2_0 != (int)((long long)(abi_mix(s, 2u) % 4096u) - 2048);
    bad |= (*p).m2_1 != (unsigned)(abi_mix(s, 3u) % 4294967296u);
    bad |= (*p).m2_2 != (int)((long long)(abi_mix(s, 4u) % 1073741824u) - 536870912);
    bad |= (*p).m2_3 != (unsigned)(abi_mix(s, 5u) % 536870912u);
    return !bad;
}
static void fill_R6(struct R6 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 512u);
    (*p).m1 = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m2 = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p).m3_0 = (int)((long long)(abi_mix(s, 4u) % 32u) - 16);
    (*p).m3_1 = (unsigned)(abi_mix(s, 5u) % 134217728u);
    (*p).m3_2 = (unsigned)(abi_mix(s, 6u) % 2097152u);
    (*p).m4 = (abi_anchor + (abi_mix(s, 7u) % 8u));
}
static int check_R6(const struct R6 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 512u);
    bad |= (*p).m1 != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m2 != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p).m3_0 != (int)((long long)(abi_mix(s, 4u) % 32u) - 16);
    bad |= (*p).m3_1 != (unsigned)(abi_mix(s, 5u) % 134217728u);
    bad |= (*p).m3_2 != (unsigned)(abi_mix(s, 6u) % 2097152u);
    bad |= (*p).m4 != (abi_anchor + (abi_mix(s, 7u) % 8u));
    return !bad;
}
static void fill_R7(struct R7 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 1024u) - 512);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 67108864u);
    (*p).m0_2 = (unsigned)(abi_mix(s, 2u) % 16384u);
    (*p).m0_3 = (int)((long long)(abi_mix(s, 3u) % 268435456u) - 134217728);
}
static int check_R7(const struct R7 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 1024u) - 512);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 67108864u);
    bad |= (*p).m0_2 != (unsigned)(abi_mix(s, 2u) % 16384u);
    bad |= (*p).m0_3 != (int)((long long)(abi_mix(s, 3u) % 268435456u) - 134217728);
    return !bad;
}
static void fill_R8(struct R8 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0_0 = (int)((long long)(abi_mix(s, 0u) % 1024u) - 512);
    (*p).m0.m0_1 = (unsigned)(abi_mix(s, 1u) % 67108864u);
    (*p).m0.m0_2 = (unsigned)(abi_mix(s, 2u) % 16384u);
    (*p).m0.m0_3 = (int)((long long)(abi_mix(s, 3u) % 268435456u) - 134217728);
    (*p).m1_0 = (unsigned)(abi_mix(s, 4u) % 16777216u);
}
static int check_R8(const struct R8 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0_0 != (int)((long long)(abi_mix(s, 0u) % 1024u) - 512);
    bad |= (*p).m0.m0_1 != (unsigned)(abi_mix(s, 1u) % 67108864u);
    bad |= (*p).m0.m0_2 != (unsigned)(abi_mix(s, 2u) % 16384u);
    bad |= (*p).m0.m0_3 != (int)((long long)(abi_mix(s, 3u) % 268435456u) - 134217728);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 4u) % 16777216u);
    return !bad;
}
static void fill_R9(struct R9 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m1.m0.m0_0 = (int)((long long)(abi_mix(s, 1u) % 1024u) - 512);
    (*p).m1.m0.m0_1 = (unsigned)(abi_mix(s, 2u) % 67108864u);
    (*p).m1.m0.m0_2 = (unsigned)(abi_mix(s, 3u) % 16384u);
    (*p).m1.m0.m0_3 = (int)((long long)(abi_mix(s, 4u) % 268435456u) - 134217728);
    (*p).m1.m1_0 = (unsigned)(abi_mix(s, 5u) % 16777216u);
}
static int check_R9(const struct R9 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m1.m0.m0_0 != (int)((long long)(abi_mix(s, 1u) % 1024u) - 512);
    bad |= (*p).m1.m0.m0_1 != (unsigned)(abi_mix(s, 2u) % 67108864u);
    bad |= (*p).m1.m0.m0_2 != (unsigned)(abi_mix(s, 3u) % 16384u);
    bad |= (*p).m1.m0.m0_3 != (int)((long long)(abi_mix(s, 4u) % 268435456u) - 134217728);
    bad |= (*p).m1.m1_0 != (unsigned)(abi_mix(s, 5u) % 16777216u);
    return !bad;
}
static void fill_R10(struct R10 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
    (*p).m1[0] = (unsigned long long)abi_mix(s, 1u);
    (*p).m1[1] = (unsigned long long)abi_mix(s, 2u);
    (*p).m1[2] = (unsigned long long)abi_mix(s, 3u);
}
static int check_R10(const struct R10 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m1[0] != (unsigned long long)abi_mix(s, 1u);
    bad |= (*p).m1[1] != (unsigned long long)abi_mix(s, 2u);
    bad |= (*p).m1[2] != (unsigned long long)abi_mix(s, 3u);
    return !bad;
}
static void fill_R11(struct R11 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m1_0 = (unsigned)(abi_mix(s, 1u) % 134217728u);
    (*p).m2 = (unsigned)(abi_mix(s, 2u) % 4294967296u);
}
static int check_R11(const struct R11 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 1u) % 134217728u);
    bad |= (*p).m2 != (unsigned)(abi_mix(s, 2u) % 4294967296u);
    return !bad;
}
static void fill_R12(union R12 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
}
static int check_R12(const union R12 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    return !bad;
}
static void fill_R13(union R13 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
}
static int check_R13(const union R13 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    return !bad;
}
static void fill_R14(struct R14 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m1_0 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m2 = (unsigned char)(abi_mix(s, 2u) % 256u);
}
static int check_R14(const struct R14 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m1_0 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m2 != (unsigned char)(abi_mix(s, 2u) % 256u);
    return !bad;
}
static void fill_R15(struct R15 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m1[0] = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    (*p).m2 = (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    (*p).m3 = (long long)abi_mix(s, 3u);
}
static int check_R15(const struct R15 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m1[0] != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    bad |= (*p).m2 != (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    bad |= (*p).m3 != (long long)abi_mix(s, 3u);
    return !bad;
}
static void fill_R16(struct R16 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 16777216u) - 8388608);
    (*p).m0_1 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m1[0] = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m1[1] = (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
}
static int check_R16(const struct R16 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 16777216u) - 8388608);
    bad |= (*p).m0_1 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m1[0] != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m1[1] != (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    return !bad;
}
static void fill_R17(struct R17 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0_0 = (int)((long long)(abi_mix(s, 0u) % 16777216u) - 8388608);
    (*p).m0.m0_1 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m0.m1[0] = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m0.m1[1] = (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (long long)abi_mix(s, 5u);
}
static int check_R17(const struct R17 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0_0 != (int)((long long)(abi_mix(s, 0u) % 16777216u) - 8388608);
    bad |= (*p).m0.m0_1 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m0.m1[0] != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m0.m1[1] != (signed char)((long long)(abi_mix(s, 3u) % 256u) - 128);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (long long)abi_mix(s, 5u);
    return !bad;
}
static void fill_R18(struct R18 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 131072u);
    (*p).m0_1 = (int)((long long)(abi_mix(s, 1u) % 32768u) - 16384);
    (*p).m0_2 = (unsigned)(abi_mix(s, 2u) % 128u);
    (*p).m0_3 = (unsigned)(abi_mix(s, 3u) % 8192u);
    (*p).m1_0 = (unsigned)(abi_mix(s, 4u) % 1048576u);
    (*p).m2 = (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    (*p).m3[0] = (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1] = (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m3[2] = (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    (*p).m3[3] = (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
}
static int check_R18(const struct R18 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 131072u);
    bad |= (*p).m0_1 != (int)((long long)(abi_mix(s, 1u) % 32768u) - 16384);
    bad |= (*p).m0_2 != (unsigned)(abi_mix(s, 2u) % 128u);
    bad |= (*p).m0_3 != (unsigned)(abi_mix(s, 3u) % 8192u);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 4u) % 1048576u);
    bad |= (*p).m2 != (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    bad |= (*p).m3[0] != (float)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1] != (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[2] != (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[3] != (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R19(struct R19 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
}
static int check_R19(const struct R19 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    return !bad;
}
static void fill_R20(struct R20 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
}
static int check_R20(const struct R20 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    return !bad;
}
static void fill_R21(struct R21 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R21(const struct R21 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R22(struct R22 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R22(const struct R22 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R23(struct R23 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R23(const struct R23 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R24(struct R24 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m0.m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m2 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R24(const struct R24 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m0.m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m2 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R25(struct R25 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
}
static int check_R25(const struct R25 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    return !bad;
}
static void fill_R26(union R26 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
}
static int check_R26(const union R26 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    return !bad;
}
static void fill_R27(struct R27 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0.m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2_0 = (int)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m2_1 = (_Bool)(abi_mix(s, 3u) & 1u);
    (*p).m2_2 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m2_3 = (unsigned)(abi_mix(s, 5u) % 32u);
}
static int check_R27(const struct R27 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0.m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2_0 != (int)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m2_1 != (_Bool)(abi_mix(s, 3u) & 1u);
    bad |= (*p).m2_2 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m2_3 != (unsigned)(abi_mix(s, 5u) % 32u);
    return !bad;
}
static void fill_R28(struct R28 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m1 = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
}
static int check_R28(const struct R28 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m1 != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R29(union R29 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0.m1 = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
}
static int check_R29(const union R29 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0.m1 != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R30(struct R30 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m1[0] = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m1[1] = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p).m1[2] = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p).m1[3] = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p).m1[4] = (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
}
static int check_R30(const struct R30 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m1[0] != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[1] != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[2] != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[3] != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[4] != (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R31(struct R31 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0.m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0.m0.m1 = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m1 = (unsigned)(abi_mix(s, 2u) % 4294967296u);
    (*p).m2 = (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
    (*p).m3.m0 = (unsigned short)(abi_mix(s, 4u) % 65536u);
    (*p).m3.m1[0] = (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    (*p).m3.m1[1] = (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    (*p).m3.m1[2] = (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
    (*p).m3.m1[3] = (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    (*p).m3.m1[4] = (int)((long long)(abi_mix(s, 9u) % 4294967296u) - 2147483648);
    (*p).m4 = (_Bool)(abi_mix(s, 10u) & 1u);
    (*p).m5_0 = (int)((long long)(abi_mix(s, 11u) % 65536u) - 32768);
    (*p).m5_1 = (unsigned)(abi_mix(s, 12u) % 2147483648u);
}
static int check_R31(const struct R31 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0.m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0.m0.m1 != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m1 != (unsigned)(abi_mix(s, 2u) % 4294967296u);
    bad |= (*p).m2 != (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
    bad |= (*p).m3.m0 != (unsigned short)(abi_mix(s, 4u) % 65536u);
    bad |= (*p).m3.m1[0] != (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    bad |= (*p).m3.m1[1] != (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    bad |= (*p).m3.m1[2] != (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
    bad |= (*p).m3.m1[3] != (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    bad |= (*p).m3.m1[4] != (int)((long long)(abi_mix(s, 9u) % 4294967296u) - 2147483648);
    bad |= (*p).m4 != (_Bool)(abi_mix(s, 10u) & 1u);
    bad |= (*p).m5_0 != (int)((long long)(abi_mix(s, 11u) % 65536u) - 32768);
    bad |= (*p).m5_1 != (unsigned)(abi_mix(s, 12u) % 2147483648u);
    return !bad;
}
static void fill_R32(struct R32 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m0[4] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (long long)abi_mix(s, 5u);
    (*p).m2 = (unsigned)(abi_mix(s, 6u) % 4294967296u);
}
static int check_R32(const struct R32 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[4] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (long long)abi_mix(s, 5u);
    bad |= (*p).m2 != (unsigned)(abi_mix(s, 6u) % 4294967296u);
    return !bad;
}
static void fill_R33(struct R33 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m0[1] = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m0[2] = (unsigned short)(abi_mix(s, 2u) % 65536u);
    (*p).m0[3] = (unsigned short)(abi_mix(s, 3u) % 65536u);
    (*p).m0[4] = (unsigned short)(abi_mix(s, 4u) % 65536u);
    (*p).m1_0 = (int)((long long)(abi_mix(s, 5u) % 4096u) - 2048);
    (*p).m1_1 = (unsigned)(abi_mix(s, 6u) % 2048u);
    (*p).m1_2 = (int)((long long)(abi_mix(s, 7u) % 4096u) - 2048);
    (*p).m2 = (unsigned char)(abi_mix(s, 8u) % 256u);
    (*p).m3[0] = (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1] = (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    (*p).m4_0 = (unsigned)(abi_mix(s, 11u) % 4u);
}
static int check_R33(const struct R33 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m0[1] != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m0[2] != (unsigned short)(abi_mix(s, 2u) % 65536u);
    bad |= (*p).m0[3] != (unsigned short)(abi_mix(s, 3u) % 65536u);
    bad |= (*p).m0[4] != (unsigned short)(abi_mix(s, 4u) % 65536u);
    bad |= (*p).m1_0 != (int)((long long)(abi_mix(s, 5u) % 4096u) - 2048);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 6u) % 2048u);
    bad |= (*p).m1_2 != (int)((long long)(abi_mix(s, 7u) % 4096u) - 2048);
    bad |= (*p).m2 != (unsigned char)(abi_mix(s, 8u) % 256u);
    bad |= (*p).m3[0] != (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1] != (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4_0 != (unsigned)(abi_mix(s, 11u) % 4u);
    return !bad;
}
static void fill_R34(struct R34 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m1.m0[0] = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m1.m0[1] = (unsigned short)(abi_mix(s, 2u) % 65536u);
    (*p).m1.m0[2] = (unsigned short)(abi_mix(s, 3u) % 65536u);
    (*p).m1.m0[3] = (unsigned short)(abi_mix(s, 4u) % 65536u);
    (*p).m1.m0[4] = (unsigned short)(abi_mix(s, 5u) % 65536u);
    (*p).m1.m1_0 = (int)((long long)(abi_mix(s, 6u) % 4096u) - 2048);
    (*p).m1.m1_1 = (unsigned)(abi_mix(s, 7u) % 2048u);
    (*p).m1.m1_2 = (int)((long long)(abi_mix(s, 8u) % 4096u) - 2048);
    (*p).m1.m2 = (unsigned char)(abi_mix(s, 9u) % 256u);
    (*p).m1.m3[0] = (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m3[1] = (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m4_0 = (unsigned)(abi_mix(s, 12u) % 4u);
}
static int check_R34(const struct R34 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m1.m0[0] != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m1.m0[1] != (unsigned short)(abi_mix(s, 2u) % 65536u);
    bad |= (*p).m1.m0[2] != (unsigned short)(abi_mix(s, 3u) % 65536u);
    bad |= (*p).m1.m0[3] != (unsigned short)(abi_mix(s, 4u) % 65536u);
    bad |= (*p).m1.m0[4] != (unsigned short)(abi_mix(s, 5u) % 65536u);
    bad |= (*p).m1.m1_0 != (int)((long long)(abi_mix(s, 6u) % 4096u) - 2048);
    bad |= (*p).m1.m1_1 != (unsigned)(abi_mix(s, 7u) % 2048u);
    bad |= (*p).m1.m1_2 != (int)((long long)(abi_mix(s, 8u) % 4096u) - 2048);
    bad |= (*p).m1.m2 != (unsigned char)(abi_mix(s, 9u) % 256u);
    bad |= (*p).m1.m3[0] != (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m3[1] != (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m4_0 != (unsigned)(abi_mix(s, 12u) % 4u);
    return !bad;
}
static void fill_R35(struct R35 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (long long)abi_mix(s, 0u);
    (*p).m1[0] = (abi_anchor + (abi_mix(s, 1u) % 8u));
}
static int check_R35(const struct R35 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (long long)abi_mix(s, 0u);
    bad |= (*p).m1[0] != (abi_anchor + (abi_mix(s, 1u) % 8u));
    return !bad;
}
static void fill_R36(struct R36 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m1 = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (unsigned char)(abi_mix(s, 3u) % 256u);
}
static int check_R36(const struct R36 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m1 != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (unsigned char)(abi_mix(s, 3u) % 256u);
    return !bad;
}
static void fill_R37(struct R37 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (long long)abi_mix(s, 0u);
    (*p).m1 = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p).m4 = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p).m5 = (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m6 = (_Bool)(abi_mix(s, 6u) & 1u);
    (*p).m7 = (unsigned char)(abi_mix(s, 7u) % 256u);
}
static int check_R37(const struct R37 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (long long)abi_mix(s, 0u);
    bad |= (*p).m1 != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p).m4 != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p).m5 != (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m6 != (_Bool)(abi_mix(s, 6u) & 1u);
    bad |= (*p).m7 != (unsigned char)(abi_mix(s, 7u) % 256u);
    return !bad;
}
static void fill_R38(struct R38 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (long long)abi_mix(s, 0u);
    (*p).m0.m1 = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    (*p).m0.m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m3 = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p).m0.m4 = (unsigned char)(abi_mix(s, 4u) % 256u);
    (*p).m0.m5 = (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m6 = (_Bool)(abi_mix(s, 6u) & 1u);
    (*p).m0.m7 = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m1 = (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    (*p).m2 = (unsigned short)(abi_mix(s, 9u) % 65536u);
}
static int check_R38(const struct R38 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (long long)abi_mix(s, 0u);
    bad |= (*p).m0.m1 != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    bad |= (*p).m0.m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m3 != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p).m0.m4 != (unsigned char)(abi_mix(s, 4u) % 256u);
    bad |= (*p).m0.m5 != (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m6 != (_Bool)(abi_mix(s, 6u) & 1u);
    bad |= (*p).m0.m7 != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m1 != (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    bad |= (*p).m2 != (unsigned short)(abi_mix(s, 9u) % 65536u);
    return !bad;
}
static void fill_R39(struct R39 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
    (*p).m1[0] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m0 = (long long)abi_mix(s, 3u);
    (*p).m2.m1[0] = (abi_anchor + (abi_mix(s, 4u) % 8u));
    (*p).m3.m0 = (unsigned char)(abi_mix(s, 5u) % 256u);
    (*p).m3.m1 = (unsigned short)(abi_mix(s, 6u) % 65536u);
    (*p).m3.m2 = (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m3.m3 = (unsigned char)(abi_mix(s, 8u) % 256u);
    (*p).m4 = (signed char)((long long)(abi_mix(s, 9u) % 256u) - 128);
    (*p).m5 = (signed char)((long long)(abi_mix(s, 10u) % 256u) - 128);
    (*p).m6.m0.m0 = (long long)abi_mix(s, 11u);
    (*p).m6.m0.m1 = (short)((long long)(abi_mix(s, 12u) % 65536u) - 32768);
    (*p).m6.m0.m2 = (float)((double)(long long)(abi_mix(s, 13u) % 4001u) / 8.0 - 250.0);
    (*p).m6.m0.m3 = (int)((long long)(abi_mix(s, 14u) % 4294967296u) - 2147483648);
    (*p).m6.m0.m4 = (unsigned char)(abi_mix(s, 15u) % 256u);
    (*p).m6.m0.m5 = (double)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
    (*p).m6.m0.m6 = (_Bool)(abi_mix(s, 17u) & 1u);
    (*p).m6.m0.m7 = (unsigned char)(abi_mix(s, 18u) % 256u);
    (*p).m6.m1 = (int)((long long)(abi_mix(s, 19u) % 4294967296u) - 2147483648);
    (*p).m6.m2 = (unsigned short)(abi_mix(s, 20u) % 65536u);
    (*p).m7 = (double)((double)(long long)(abi_mix(s, 21u) % 4001u) / 8.0 - 250.0);
}
static int check_R39(const struct R39 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m1[0] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m0 != (long long)abi_mix(s, 3u);
    bad |= (*p).m2.m1[0] != (abi_anchor + (abi_mix(s, 4u) % 8u));
    bad |= (*p).m3.m0 != (unsigned char)(abi_mix(s, 5u) % 256u);
    bad |= (*p).m3.m1 != (unsigned short)(abi_mix(s, 6u) % 65536u);
    bad |= (*p).m3.m2 != (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3.m3 != (unsigned char)(abi_mix(s, 8u) % 256u);
    bad |= (*p).m4 != (signed char)((long long)(abi_mix(s, 9u) % 256u) - 128);
    bad |= (*p).m5 != (signed char)((long long)(abi_mix(s, 10u) % 256u) - 128);
    bad |= (*p).m6.m0.m0 != (long long)abi_mix(s, 11u);
    bad |= (*p).m6.m0.m1 != (short)((long long)(abi_mix(s, 12u) % 65536u) - 32768);
    bad |= (*p).m6.m0.m2 != (float)((double)(long long)(abi_mix(s, 13u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m6.m0.m3 != (int)((long long)(abi_mix(s, 14u) % 4294967296u) - 2147483648);
    bad |= (*p).m6.m0.m4 != (unsigned char)(abi_mix(s, 15u) % 256u);
    bad |= (*p).m6.m0.m5 != (double)((double)(long long)(abi_mix(s, 16u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m6.m0.m6 != (_Bool)(abi_mix(s, 17u) & 1u);
    bad |= (*p).m6.m0.m7 != (unsigned char)(abi_mix(s, 18u) % 256u);
    bad |= (*p).m6.m1 != (int)((long long)(abi_mix(s, 19u) % 4294967296u) - 2147483648);
    bad |= (*p).m6.m2 != (unsigned short)(abi_mix(s, 20u) % 65536u);
    bad |= (*p).m7 != (double)((double)(long long)(abi_mix(s, 21u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R40(struct R40 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m0[1] = (_Bool)(abi_mix(s, 1u) & 1u);
}
static int check_R40(const struct R40 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m0[1] != (_Bool)(abi_mix(s, 1u) & 1u);
    return !bad;
}
static void fill_R41(union R41 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
}
static int check_R41(const union R41 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    return !bad;
}
static void fill_R42(struct R42 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R42(const struct R42 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R43(struct R43 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R43(const struct R43 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R44(struct R44 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 16u);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 134217728u);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 1073741824u) - 536870912);
}
static int check_R44(const struct R44 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 16u);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 134217728u);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 1073741824u) - 536870912);
    return !bad;
}
static void fill_R45(struct R45 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
    (*p).m1 = (unsigned short)(abi_mix(s, 1u) % 65536u);
}
static int check_R45(const struct R45 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    bad |= (*p).m1 != (unsigned short)(abi_mix(s, 1u) % 65536u);
    return !bad;
}
static void fill_R46(struct R46 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p).m0[1] = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m0[2] = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p).m0[3] = (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    (*p).m1.m0 = (abi_anchor + (abi_mix(s, 4u) % 8u));
    (*p).m1.m1 = (unsigned short)(abi_mix(s, 5u) % 65536u);
    (*p).m2 = (unsigned char)(abi_mix(s, 6u) % 256u);
    (*p).m3 = (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
}
static int check_R46(const struct R46 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[1] != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[2] != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p).m0[3] != (int)((long long)(abi_mix(s, 3u) % 4294967296u) - 2147483648);
    bad |= (*p).m1.m0 != (abi_anchor + (abi_mix(s, 4u) % 8u));
    bad |= (*p).m1.m1 != (unsigned short)(abi_mix(s, 5u) % 65536u);
    bad |= (*p).m2 != (unsigned char)(abi_mix(s, 6u) % 256u);
    bad |= (*p).m3 != (int)((long long)(abi_mix(s, 7u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R47(union R47 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
}
static int check_R47(const union R47 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    return !bad;
}
static void fill_R48(struct R48 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
}
static int check_R48(const struct R48 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    return !bad;
}
static void fill_R49(struct R49 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R49(const struct R49 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R50(struct R50 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 128u);
    (*p).m0_1 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m1 = (unsigned)(abi_mix(s, 2u) % 4294967296u);
    (*p).m2 = (abi_anchor + (abi_mix(s, 3u) % 8u));
}
static int check_R50(const struct R50 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 128u);
    bad |= (*p).m0_1 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m1 != (unsigned)(abi_mix(s, 2u) % 4294967296u);
    bad |= (*p).m2 != (abi_anchor + (abi_mix(s, 3u) % 8u));
    return !bad;
}
static void fill_R51(struct R51 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
    (*p).m1 = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m2_0 = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m2_1 = (_Bool)(abi_mix(s, 3u) & 1u);
    (*p).m2_2 = (unsigned)(abi_mix(s, 4u) % 256u);
}
static int check_R51(const struct R51 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    bad |= (*p).m1 != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m2_0 != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m2_1 != (_Bool)(abi_mix(s, 3u) & 1u);
    bad |= (*p).m2_2 != (unsigned)(abi_mix(s, 4u) % 256u);
    return !bad;
}
static void fill_R52(struct R52 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (long long)abi_mix(s, 0u);
    (*p).m1 = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m2 = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m3[0] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1] = (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m3[2] = (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m3[3] = (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m3[4] = (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m4[0] = (unsigned char)(abi_mix(s, 8u) % 256u);
}
static int check_R52(const struct R52 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (long long)abi_mix(s, 0u);
    bad |= (*p).m1 != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m2 != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m3[0] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1] != (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[2] != (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[3] != (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[4] != (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[0] != (unsigned char)(abi_mix(s, 8u) % 256u);
    return !bad;
}
static void fill_R53(union R53 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R53(const union R53 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R54(struct R54 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
    (*p).m1 = (abi_anchor + (abi_mix(s, 1u) % 8u));
}
static int check_R54(const struct R54 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    bad |= (*p).m1 != (abi_anchor + (abi_mix(s, 1u) % 8u));
    return !bad;
}
static void fill_R55(struct R55 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 16777216u);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 134217728u);
    (*p).m1 = (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    (*p).m2 = (unsigned short)(abi_mix(s, 3u) % 65536u);
    (*p).m3 = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m4 = (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    (*p).m5[0] = (abi_anchor + (abi_mix(s, 6u) % 8u));
    (*p).m5[1] = (abi_anchor + (abi_mix(s, 7u) % 8u));
    (*p).m5[2] = (abi_anchor + (abi_mix(s, 8u) % 8u));
}
static int check_R55(const struct R55 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 16777216u);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 134217728u);
    bad |= (*p).m1 != (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    bad |= (*p).m2 != (unsigned short)(abi_mix(s, 3u) % 65536u);
    bad |= (*p).m3 != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4 != (signed char)((long long)(abi_mix(s, 5u) % 256u) - 128);
    bad |= (*p).m5[0] != (abi_anchor + (abi_mix(s, 6u) % 8u));
    bad |= (*p).m5[1] != (abi_anchor + (abi_mix(s, 7u) % 8u));
    bad |= (*p).m5[2] != (abi_anchor + (abi_mix(s, 8u) % 8u));
    return !bad;
}
static void fill_R56(struct R56 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m1 = (long long)abi_mix(s, 1u);
}
static int check_R56(const struct R56 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m1 != (long long)abi_mix(s, 1u);
    return !bad;
}
static void fill_R57(struct R57 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0.m1 = (long long)abi_mix(s, 1u);
    (*p).m1 = (long long)abi_mix(s, 2u);
}
static int check_R57(const struct R57 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0.m1 != (long long)abi_mix(s, 1u);
    bad |= (*p).m1 != (long long)abi_mix(s, 2u);
    return !bad;
}
static void fill_R58(struct R58 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 1024u);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 262144u);
    (*p).m0_2 = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m0_3 = (_Bool)(abi_mix(s, 3u) & 1u);
}
static int check_R58(const struct R58 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 1024u);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 262144u);
    bad |= (*p).m0_2 != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m0_3 != (_Bool)(abi_mix(s, 3u) & 1u);
    return !bad;
}
static void fill_R59(struct R59 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R59(const struct R59 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}

float abi_f0(float a0, unsigned short a1, struct R0 a2, struct R1 a3, unsigned a4, signed char a5, struct R2 a6, double a7, struct R3 a8, struct R4 a9);
_Bool abi_f1(struct R5 a0, struct R6 a1, int a2, int a3, struct R9 a4);
struct R10 abi_f2(union R13 a0, struct R14 a1, struct R15 a2, struct R17 a3, float a4);
struct R18 abi_f3(struct R19 a0, struct R20 a1, struct R22 a2, struct R24 a3, struct R27 a4);
struct R31 abi_f4(struct R32 a0, struct R34 a1, struct R39 a2, struct R40 a3, ...);
void abi_f5(union R41 a0, unsigned char a1, struct R43 a2);
struct R44 abi_f6(union R47 a0, struct R48 a1, _Bool a2, struct R49 a3, struct R50 a4, float a5, char * a6, struct R51 a7, struct R52 a8, double a9);
union R53 abi_f7(double a0, struct R54 a1, struct R55 a2, unsigned a3, signed char a4, signed char a5, struct R57 a6, struct R58 a7, struct R59 a8, unsigned char a9);
#endif
