/* abi_gen_104.h - written by gen_c_abi.py 104 8: the types of the
   tests in abi_gen_104.c and abi_gen_104_callee.c, and the functions that fill
   and check them. */
#ifndef ABI_GEN_104_H
#define ABI_GEN_104_H
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
};
struct R1 {
    unsigned _BitInt(129) m0;
    int : 0;
    unsigned m1_0 : 29;
    int m1_1 : 5;
};
struct R2 {
    float m0;
    float m1[2];
    float m2;
    float m3;
};
struct R3 {
    double m0;
};
struct R4 {
    struct R3 m0;
    double m1[2];
    double m2;
    double m3;
};
struct R5 {
    int m0_0 : 8;
    unsigned m0_1 : 7;
    unsigned : 3;
    int m0_2 : 9;
};
struct R6 {
    double m0[1];
    double m1;
};
struct R7 {
    unsigned char m0[4];
    __int128 m1[3];
    unsigned short m2[3];
    int m3;
    unsigned _BitInt(129) m4;
};
union R8 {
    short m0;
};
struct R9 {
    union R8 m0;
};
struct R10 {
    _Alignas(16) long long m0;
};
struct R11 {
    struct R10 m0;
};
union R12 {
    struct R7 m0;
    unsigned char m1[3];
    short m2;
    int m3[2];
    _Alignas(16) struct R9 m4[5];
    struct R11 m5;
};
struct R13 {
    unsigned m0;
};
struct R14 {
    int m0_0 : 26;
    unsigned m0_1 : 31;
    unsigned m0_2 : 3;
};
struct R15 {
    __int128 m0[1];
    _BitInt(192) m1;
};
union R16 {
    struct R15 m0;
    double m1;
};
struct R17 {
    unsigned m0;
};
struct R18 {
    _Alignas(64) unsigned long long m0;
    _BitInt(128) m1;
    unsigned m2_0 : 30;
    float m3[2];
    short m4;
    _BitInt(192) m5[2];
    struct R17 m6[4];
    unsigned _BitInt(129) m7;
};
struct R19 {
    unsigned m0_0 : 7;
    int m0_1 : 26;
    int m0_2 : 11;
};
struct R20 {
    __int128 m0;
    unsigned __int128 m1;
};
union R21 {
    _BitInt(65) m0;
};
struct R22 {
    _Bool m0_0 : 1;
    unsigned m0_1 : 29;
    int m0_2 : 9;
    int m1_0 : 4;
    unsigned m1_1 : 17;
    unsigned m1_2 : 30;
    int m1_3 : 29;
    double m2;
};
struct R23 {
    short m0;
    int m1;
    unsigned _BitInt(129) m2;
    unsigned _BitInt(129) m3;
    unsigned _BitInt(256) m4;
};
struct R24 {
    double m0;
    double m1;
    double m2;
};
struct R25 {
    struct R23 m0;
    struct R24 m1;
};
struct R26 {
    _BitInt(65) m0[5];
    _Alignas(32) struct R22 m1;
    unsigned short m2[4];
    char * m3[3];
    struct R25 m4[5];
    unsigned m5_0 : 29;
    _BitInt(65) m6;
    signed char m7;
};
struct R27 {
    float m0[5];
};
struct R28 {
    _Alignas(64) unsigned long long m0[5];
};
struct R29 {
    _Bool m0;
    signed char m1;
    long long m2;
};
union R30 {
    unsigned short m0;
};
struct R31 {
    double m0;
    double m1;
    double m2;
};
struct R32 {
    float m0;
    float m1;
};
struct R33 {
    float m0;
    float m1;
    struct R32 m2;
};
struct R34 {
    unsigned _BitInt(256) m0;
};
struct R35 {
    int : 0;
    int m0_0 : 21;
    _Bool m0_1 : 1;
    int m0_2 : 24;
};
struct R36 {
    unsigned char m0[3];
};
struct R37 {
    unsigned _BitInt(129) m0;
};
struct R38 {
    unsigned char m0;
    _Bool m1;
};
struct R39 {
    unsigned char m0;
    unsigned long long m1;
};
struct R40 {
    int m0_0 : 27;
};
union R41 {
    short m0[4];
    unsigned m1[2];
};

static void fill_R0(struct R0 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R0(const struct R0 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R1(struct R1 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 0u) ^ ((unsigned _BitInt(129))abi_mix(s, 0u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 0u + 3000u) << 65));
    (*p).m1_0 = (unsigned)(abi_mix(s, 1u) % 536870912u);
    (*p).m1_1 = (int)((long long)(abi_mix(s, 2u) % 32u) - 16);
}
static int check_R1(const struct R1 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 0u) ^ ((unsigned _BitInt(129))abi_mix(s, 0u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 0u + 3000u) << 65));
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 1u) % 536870912u);
    bad |= (*p).m1_1 != (int)((long long)(abi_mix(s, 2u) % 32u) - 16);
    return !bad;
}
static void fill_R2(struct R2 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R2(const struct R2 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R3(struct R3 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R3(const struct R3 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R4(struct R4 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1[1] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R4(const struct R4 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[1] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (double)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R5(struct R5 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 128u);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 512u) - 256);
}
static int check_R5(const struct R5 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 128u);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 512u) - 256);
    return !bad;
}
static void fill_R6(struct R6 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R6(const struct R6 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R7(struct R7 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m0[1] = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m0[2] = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p).m0[3] = (unsigned char)(abi_mix(s, 3u) % 256u);
    (*p).m1[0] = (__int128)((unsigned __int128)abi_mix(s, 4u) ^ ((unsigned __int128)abi_mix(s, 4u + 2000u) << 64));
    (*p).m1[1] = (__int128)((unsigned __int128)abi_mix(s, 5u) ^ ((unsigned __int128)abi_mix(s, 5u + 2000u) << 64));
    (*p).m1[2] = (__int128)((unsigned __int128)abi_mix(s, 6u) ^ ((unsigned __int128)abi_mix(s, 6u + 2000u) << 64));
    (*p).m2[0] = (unsigned short)(abi_mix(s, 7u) % 65536u);
    (*p).m2[1] = (unsigned short)(abi_mix(s, 8u) % 65536u);
    (*p).m2[2] = (unsigned short)(abi_mix(s, 9u) % 65536u);
    (*p).m3 = (int)((long long)(abi_mix(s, 10u) % 4294967296u) - 2147483648);
    (*p).m4 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 11u) ^ ((unsigned _BitInt(129))abi_mix(s, 11u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 11u + 3000u) << 65));
}
static int check_R7(const struct R7 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m0[1] != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m0[2] != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m0[3] != (unsigned char)(abi_mix(s, 3u) % 256u);
    bad |= (*p).m1[0] != (__int128)((unsigned __int128)abi_mix(s, 4u) ^ ((unsigned __int128)abi_mix(s, 4u + 2000u) << 64));
    bad |= (*p).m1[1] != (__int128)((unsigned __int128)abi_mix(s, 5u) ^ ((unsigned __int128)abi_mix(s, 5u + 2000u) << 64));
    bad |= (*p).m1[2] != (__int128)((unsigned __int128)abi_mix(s, 6u) ^ ((unsigned __int128)abi_mix(s, 6u + 2000u) << 64));
    bad |= (*p).m2[0] != (unsigned short)(abi_mix(s, 7u) % 65536u);
    bad |= (*p).m2[1] != (unsigned short)(abi_mix(s, 8u) % 65536u);
    bad |= (*p).m2[2] != (unsigned short)(abi_mix(s, 9u) % 65536u);
    bad |= (*p).m3 != (int)((long long)(abi_mix(s, 10u) % 4294967296u) - 2147483648);
    bad |= (*p).m4 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 11u) ^ ((unsigned _BitInt(129))abi_mix(s, 11u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 11u + 3000u) << 65));
    return !bad;
}
static void fill_R8(union R8 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
}
static int check_R8(const union R8 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    return !bad;
}
static void fill_R9(struct R9 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
}
static int check_R9(const struct R9 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    return !bad;
}
static void fill_R10(struct R10 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (long long)abi_mix(s, 0u);
}
static int check_R10(const struct R10 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (long long)abi_mix(s, 0u);
    return !bad;
}
static void fill_R11(struct R11 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (long long)abi_mix(s, 0u);
}
static int check_R11(const struct R11 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (long long)abi_mix(s, 0u);
    return !bad;
}
static void fill_R12(union R12 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0[0] = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m0.m0[1] = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m0.m0[2] = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p).m0.m0[3] = (unsigned char)(abi_mix(s, 3u) % 256u);
    (*p).m0.m1[0] = (__int128)((unsigned __int128)abi_mix(s, 4u) ^ ((unsigned __int128)abi_mix(s, 4u + 2000u) << 64));
    (*p).m0.m1[1] = (__int128)((unsigned __int128)abi_mix(s, 5u) ^ ((unsigned __int128)abi_mix(s, 5u + 2000u) << 64));
    (*p).m0.m1[2] = (__int128)((unsigned __int128)abi_mix(s, 6u) ^ ((unsigned __int128)abi_mix(s, 6u + 2000u) << 64));
    (*p).m0.m2[0] = (unsigned short)(abi_mix(s, 7u) % 65536u);
    (*p).m0.m2[1] = (unsigned short)(abi_mix(s, 8u) % 65536u);
    (*p).m0.m2[2] = (unsigned short)(abi_mix(s, 9u) % 65536u);
    (*p).m0.m3 = (int)((long long)(abi_mix(s, 10u) % 4294967296u) - 2147483648);
    (*p).m0.m4 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 11u) ^ ((unsigned _BitInt(129))abi_mix(s, 11u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 11u + 3000u) << 65));
}
static int check_R12(const union R12 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0[0] != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m0.m0[1] != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m0.m0[2] != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m0.m0[3] != (unsigned char)(abi_mix(s, 3u) % 256u);
    bad |= (*p).m0.m1[0] != (__int128)((unsigned __int128)abi_mix(s, 4u) ^ ((unsigned __int128)abi_mix(s, 4u + 2000u) << 64));
    bad |= (*p).m0.m1[1] != (__int128)((unsigned __int128)abi_mix(s, 5u) ^ ((unsigned __int128)abi_mix(s, 5u + 2000u) << 64));
    bad |= (*p).m0.m1[2] != (__int128)((unsigned __int128)abi_mix(s, 6u) ^ ((unsigned __int128)abi_mix(s, 6u + 2000u) << 64));
    bad |= (*p).m0.m2[0] != (unsigned short)(abi_mix(s, 7u) % 65536u);
    bad |= (*p).m0.m2[1] != (unsigned short)(abi_mix(s, 8u) % 65536u);
    bad |= (*p).m0.m2[2] != (unsigned short)(abi_mix(s, 9u) % 65536u);
    bad |= (*p).m0.m3 != (int)((long long)(abi_mix(s, 10u) % 4294967296u) - 2147483648);
    bad |= (*p).m0.m4 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 11u) ^ ((unsigned _BitInt(129))abi_mix(s, 11u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 11u + 3000u) << 65));
    return !bad;
}
static void fill_R13(struct R13 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
}
static int check_R13(const struct R13 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    return !bad;
}
static void fill_R14(struct R14 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 67108864u) - 33554432);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 2147483648u);
    (*p).m0_2 = (unsigned)(abi_mix(s, 2u) % 8u);
}
static int check_R14(const struct R14 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 67108864u) - 33554432);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 2147483648u);
    bad |= (*p).m0_2 != (unsigned)(abi_mix(s, 2u) % 8u);
    return !bad;
}
static void fill_R15(struct R15 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (__int128)((unsigned __int128)abi_mix(s, 0u) ^ ((unsigned __int128)abi_mix(s, 0u + 2000u) << 64));
    (*p).m1 = (_BitInt(192))((unsigned _BitInt(192))abi_mix(s, 1u) ^ ((unsigned _BitInt(192))abi_mix(s, 1u + 2000u) << 64) ^ ((unsigned _BitInt(192))abi_mix(s, 1u + 3000u) << 128));
}
static int check_R15(const struct R15 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (__int128)((unsigned __int128)abi_mix(s, 0u) ^ ((unsigned __int128)abi_mix(s, 0u + 2000u) << 64));
    bad |= (*p).m1 != (_BitInt(192))((unsigned _BitInt(192))abi_mix(s, 1u) ^ ((unsigned _BitInt(192))abi_mix(s, 1u + 2000u) << 64) ^ ((unsigned _BitInt(192))abi_mix(s, 1u + 3000u) << 128));
    return !bad;
}
static void fill_R16(union R16 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0[0] = (__int128)((unsigned __int128)abi_mix(s, 0u) ^ ((unsigned __int128)abi_mix(s, 0u + 2000u) << 64));
    (*p).m0.m1 = (_BitInt(192))((unsigned _BitInt(192))abi_mix(s, 1u) ^ ((unsigned _BitInt(192))abi_mix(s, 1u + 2000u) << 64) ^ ((unsigned _BitInt(192))abi_mix(s, 1u + 3000u) << 128));
}
static int check_R16(const union R16 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0[0] != (__int128)((unsigned __int128)abi_mix(s, 0u) ^ ((unsigned __int128)abi_mix(s, 0u + 2000u) << 64));
    bad |= (*p).m0.m1 != (_BitInt(192))((unsigned _BitInt(192))abi_mix(s, 1u) ^ ((unsigned _BitInt(192))abi_mix(s, 1u + 2000u) << 64) ^ ((unsigned _BitInt(192))abi_mix(s, 1u + 3000u) << 128));
    return !bad;
}
static void fill_R17(struct R17 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
}
static int check_R17(const struct R17 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    return !bad;
}
static void fill_R18(struct R18 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
    (*p).m1 = (_BitInt(128))((unsigned _BitInt(128))abi_mix(s, 1u) ^ ((unsigned _BitInt(128))abi_mix(s, 1u + 2000u) << 64));
    (*p).m2_0 = (unsigned)(abi_mix(s, 2u) % 1073741824u);
    (*p).m3[0] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    (*p).m4 = (short)((long long)(abi_mix(s, 5u) % 65536u) - 32768);
    (*p).m5[0] = (_BitInt(192))((unsigned _BitInt(192))abi_mix(s, 6u) ^ ((unsigned _BitInt(192))abi_mix(s, 6u + 2000u) << 64) ^ ((unsigned _BitInt(192))abi_mix(s, 6u + 3000u) << 128));
    (*p).m5[1] = (_BitInt(192))((unsigned _BitInt(192))abi_mix(s, 7u) ^ ((unsigned _BitInt(192))abi_mix(s, 7u + 2000u) << 64) ^ ((unsigned _BitInt(192))abi_mix(s, 7u + 3000u) << 128));
    (*p).m6[0].m0 = (unsigned)(abi_mix(s, 8u) % 4294967296u);
    (*p).m6[1].m0 = (unsigned)(abi_mix(s, 9u) % 4294967296u);
    (*p).m6[2].m0 = (unsigned)(abi_mix(s, 10u) % 4294967296u);
    (*p).m6[3].m0 = (unsigned)(abi_mix(s, 11u) % 4294967296u);
    (*p).m7 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 12u) ^ ((unsigned _BitInt(129))abi_mix(s, 12u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 12u + 3000u) << 65));
}
static int check_R18(const struct R18 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m1 != (_BitInt(128))((unsigned _BitInt(128))abi_mix(s, 1u) ^ ((unsigned _BitInt(128))abi_mix(s, 1u + 2000u) << 64));
    bad |= (*p).m2_0 != (unsigned)(abi_mix(s, 2u) % 1073741824u);
    bad |= (*p).m3[0] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4 != (short)((long long)(abi_mix(s, 5u) % 65536u) - 32768);
    bad |= (*p).m5[0] != (_BitInt(192))((unsigned _BitInt(192))abi_mix(s, 6u) ^ ((unsigned _BitInt(192))abi_mix(s, 6u + 2000u) << 64) ^ ((unsigned _BitInt(192))abi_mix(s, 6u + 3000u) << 128));
    bad |= (*p).m5[1] != (_BitInt(192))((unsigned _BitInt(192))abi_mix(s, 7u) ^ ((unsigned _BitInt(192))abi_mix(s, 7u + 2000u) << 64) ^ ((unsigned _BitInt(192))abi_mix(s, 7u + 3000u) << 128));
    bad |= (*p).m6[0].m0 != (unsigned)(abi_mix(s, 8u) % 4294967296u);
    bad |= (*p).m6[1].m0 != (unsigned)(abi_mix(s, 9u) % 4294967296u);
    bad |= (*p).m6[2].m0 != (unsigned)(abi_mix(s, 10u) % 4294967296u);
    bad |= (*p).m6[3].m0 != (unsigned)(abi_mix(s, 11u) % 4294967296u);
    bad |= (*p).m7 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 12u) ^ ((unsigned _BitInt(129))abi_mix(s, 12u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 12u + 3000u) << 65));
    return !bad;
}
static void fill_R19(struct R19 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (unsigned)(abi_mix(s, 0u) % 128u);
    (*p).m0_1 = (int)((long long)(abi_mix(s, 1u) % 67108864u) - 33554432);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 2048u) - 1024);
}
static int check_R19(const struct R19 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (unsigned)(abi_mix(s, 0u) % 128u);
    bad |= (*p).m0_1 != (int)((long long)(abi_mix(s, 1u) % 67108864u) - 33554432);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 2048u) - 1024);
    return !bad;
}
static void fill_R20(struct R20 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (__int128)((unsigned __int128)abi_mix(s, 0u) ^ ((unsigned __int128)abi_mix(s, 0u + 2000u) << 64));
    (*p).m1 = (unsigned __int128)((unsigned __int128)abi_mix(s, 1u) ^ ((unsigned __int128)abi_mix(s, 1u + 2000u) << 64));
}
static int check_R20(const struct R20 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (__int128)((unsigned __int128)abi_mix(s, 0u) ^ ((unsigned __int128)abi_mix(s, 0u + 2000u) << 64));
    bad |= (*p).m1 != (unsigned __int128)((unsigned __int128)abi_mix(s, 1u) ^ ((unsigned __int128)abi_mix(s, 1u + 2000u) << 64));
    return !bad;
}
static void fill_R21(union R21 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 0u) ^ ((unsigned _BitInt(65))abi_mix(s, 0u + 2000u) << 64));
}
static int check_R21(const union R21 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 0u) ^ ((unsigned _BitInt(65))abi_mix(s, 0u + 2000u) << 64));
    return !bad;
}
static void fill_R22(struct R22 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m0_1 = (unsigned)(abi_mix(s, 1u) % 536870912u);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 512u) - 256);
    (*p).m1_0 = (int)((long long)(abi_mix(s, 3u) % 16u) - 8);
    (*p).m1_1 = (unsigned)(abi_mix(s, 4u) % 131072u);
    (*p).m1_2 = (unsigned)(abi_mix(s, 5u) % 1073741824u);
    (*p).m1_3 = (int)((long long)(abi_mix(s, 6u) % 536870912u) - 268435456);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
}
static int check_R22(const struct R22 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m0_1 != (unsigned)(abi_mix(s, 1u) % 536870912u);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 512u) - 256);
    bad |= (*p).m1_0 != (int)((long long)(abi_mix(s, 3u) % 16u) - 8);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 4u) % 131072u);
    bad |= (*p).m1_2 != (unsigned)(abi_mix(s, 5u) % 1073741824u);
    bad |= (*p).m1_3 != (int)((long long)(abi_mix(s, 6u) % 536870912u) - 268435456);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R23(struct R23 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m1 = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m2 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 2u) ^ ((unsigned _BitInt(129))abi_mix(s, 2u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 2u + 3000u) << 65));
    (*p).m3 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 3u) ^ ((unsigned _BitInt(129))abi_mix(s, 3u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 3u + 3000u) << 65));
    (*p).m4 = (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 4u) ^ ((unsigned _BitInt(256))abi_mix(s, 4u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 4u + 3000u) << 192));
}
static int check_R23(const struct R23 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m1 != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m2 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 2u) ^ ((unsigned _BitInt(129))abi_mix(s, 2u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 2u + 3000u) << 65));
    bad |= (*p).m3 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 3u) ^ ((unsigned _BitInt(129))abi_mix(s, 3u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 3u + 3000u) << 65));
    bad |= (*p).m4 != (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 4u) ^ ((unsigned _BitInt(256))abi_mix(s, 4u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 4u + 3000u) << 192));
    return !bad;
}
static void fill_R24(struct R24 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R24(const struct R24 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R25(struct R25 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m0.m1 = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m0.m2 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 2u) ^ ((unsigned _BitInt(129))abi_mix(s, 2u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 2u + 3000u) << 65));
    (*p).m0.m3 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 3u) ^ ((unsigned _BitInt(129))abi_mix(s, 3u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 3u + 3000u) << 65));
    (*p).m0.m4 = (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 4u) ^ ((unsigned _BitInt(256))abi_mix(s, 4u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 4u + 3000u) << 192));
    (*p).m1.m0 = (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1 = (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2 = (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
}
static int check_R25(const struct R25 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m0.m1 != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m0.m2 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 2u) ^ ((unsigned _BitInt(129))abi_mix(s, 2u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 2u + 3000u) << 65));
    bad |= (*p).m0.m3 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 3u) ^ ((unsigned _BitInt(129))abi_mix(s, 3u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 3u + 3000u) << 65));
    bad |= (*p).m0.m4 != (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 4u) ^ ((unsigned _BitInt(256))abi_mix(s, 4u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 4u + 3000u) << 192));
    bad |= (*p).m1.m0 != (double)((double)(long long)(abi_mix(s, 5u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1 != (double)((double)(long long)(abi_mix(s, 6u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2 != (double)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R26(struct R26 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 0u) ^ ((unsigned _BitInt(65))abi_mix(s, 0u + 2000u) << 64));
    (*p).m0[1] = (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 1u) ^ ((unsigned _BitInt(65))abi_mix(s, 1u + 2000u) << 64));
    (*p).m0[2] = (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 2u) ^ ((unsigned _BitInt(65))abi_mix(s, 2u + 2000u) << 64));
    (*p).m0[3] = (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 3u) ^ ((unsigned _BitInt(65))abi_mix(s, 3u + 2000u) << 64));
    (*p).m0[4] = (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 4u) ^ ((unsigned _BitInt(65))abi_mix(s, 4u + 2000u) << 64));
    (*p).m1.m0_0 = (_Bool)(abi_mix(s, 5u) & 1u);
    (*p).m1.m0_1 = (unsigned)(abi_mix(s, 6u) % 536870912u);
    (*p).m1.m0_2 = (int)((long long)(abi_mix(s, 7u) % 512u) - 256);
    (*p).m1.m1_0 = (int)((long long)(abi_mix(s, 8u) % 16u) - 8);
    (*p).m1.m1_1 = (unsigned)(abi_mix(s, 9u) % 131072u);
    (*p).m1.m1_2 = (unsigned)(abi_mix(s, 10u) % 1073741824u);
    (*p).m1.m1_3 = (int)((long long)(abi_mix(s, 11u) % 536870912u) - 268435456);
    (*p).m1.m2 = (double)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    (*p).m2[0] = (unsigned short)(abi_mix(s, 13u) % 65536u);
    (*p).m2[1] = (unsigned short)(abi_mix(s, 14u) % 65536u);
    (*p).m2[2] = (unsigned short)(abi_mix(s, 15u) % 65536u);
    (*p).m2[3] = (unsigned short)(abi_mix(s, 16u) % 65536u);
    (*p).m3[0] = (abi_anchor + (abi_mix(s, 17u) % 8u));
    (*p).m3[1] = (abi_anchor + (abi_mix(s, 18u) % 8u));
    (*p).m3[2] = (abi_anchor + (abi_mix(s, 19u) % 8u));
    (*p).m4[0].m0.m0 = (short)((long long)(abi_mix(s, 20u) % 65536u) - 32768);
    (*p).m4[0].m0.m1 = (int)((long long)(abi_mix(s, 21u) % 4294967296u) - 2147483648);
    (*p).m4[0].m0.m2 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 22u) ^ ((unsigned _BitInt(129))abi_mix(s, 22u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 22u + 3000u) << 65));
    (*p).m4[0].m0.m3 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 23u) ^ ((unsigned _BitInt(129))abi_mix(s, 23u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 23u + 3000u) << 65));
    (*p).m4[0].m0.m4 = (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 24u) ^ ((unsigned _BitInt(256))abi_mix(s, 24u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 24u + 3000u) << 192));
    (*p).m4[0].m1.m0 = (double)((double)(long long)(abi_mix(s, 25u) % 4001u) / 8.0 - 250.0);
    (*p).m4[0].m1.m1 = (double)((double)(long long)(abi_mix(s, 26u) % 4001u) / 8.0 - 250.0);
    (*p).m4[0].m1.m2 = (double)((double)(long long)(abi_mix(s, 27u) % 4001u) / 8.0 - 250.0);
    (*p).m4[1].m0.m0 = (short)((long long)(abi_mix(s, 28u) % 65536u) - 32768);
    (*p).m4[1].m0.m1 = (int)((long long)(abi_mix(s, 29u) % 4294967296u) - 2147483648);
    (*p).m4[1].m0.m2 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 30u) ^ ((unsigned _BitInt(129))abi_mix(s, 30u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 30u + 3000u) << 65));
    (*p).m4[1].m0.m3 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 31u) ^ ((unsigned _BitInt(129))abi_mix(s, 31u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 31u + 3000u) << 65));
    (*p).m4[1].m0.m4 = (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 32u) ^ ((unsigned _BitInt(256))abi_mix(s, 32u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 32u + 3000u) << 192));
    (*p).m4[1].m1.m0 = (double)((double)(long long)(abi_mix(s, 33u) % 4001u) / 8.0 - 250.0);
    (*p).m4[1].m1.m1 = (double)((double)(long long)(abi_mix(s, 34u) % 4001u) / 8.0 - 250.0);
    (*p).m4[1].m1.m2 = (double)((double)(long long)(abi_mix(s, 35u) % 4001u) / 8.0 - 250.0);
    (*p).m4[2].m0.m0 = (short)((long long)(abi_mix(s, 36u) % 65536u) - 32768);
    (*p).m4[2].m0.m1 = (int)((long long)(abi_mix(s, 37u) % 4294967296u) - 2147483648);
    (*p).m4[2].m0.m2 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 38u) ^ ((unsigned _BitInt(129))abi_mix(s, 38u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 38u + 3000u) << 65));
    (*p).m4[2].m0.m3 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 39u) ^ ((unsigned _BitInt(129))abi_mix(s, 39u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 39u + 3000u) << 65));
    (*p).m4[2].m0.m4 = (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 40u) ^ ((unsigned _BitInt(256))abi_mix(s, 40u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 40u + 3000u) << 192));
    (*p).m4[2].m1.m0 = (double)((double)(long long)(abi_mix(s, 41u) % 4001u) / 8.0 - 250.0);
    (*p).m4[2].m1.m1 = (double)((double)(long long)(abi_mix(s, 42u) % 4001u) / 8.0 - 250.0);
    (*p).m4[2].m1.m2 = (double)((double)(long long)(abi_mix(s, 43u) % 4001u) / 8.0 - 250.0);
    (*p).m4[3].m0.m0 = (short)((long long)(abi_mix(s, 44u) % 65536u) - 32768);
    (*p).m4[3].m0.m1 = (int)((long long)(abi_mix(s, 45u) % 4294967296u) - 2147483648);
    (*p).m4[3].m0.m2 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 46u) ^ ((unsigned _BitInt(129))abi_mix(s, 46u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 46u + 3000u) << 65));
    (*p).m4[3].m0.m3 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 47u) ^ ((unsigned _BitInt(129))abi_mix(s, 47u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 47u + 3000u) << 65));
    (*p).m4[3].m0.m4 = (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 48u) ^ ((unsigned _BitInt(256))abi_mix(s, 48u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 48u + 3000u) << 192));
    (*p).m4[3].m1.m0 = (double)((double)(long long)(abi_mix(s, 49u) % 4001u) / 8.0 - 250.0);
    (*p).m4[3].m1.m1 = (double)((double)(long long)(abi_mix(s, 50u) % 4001u) / 8.0 - 250.0);
    (*p).m4[3].m1.m2 = (double)((double)(long long)(abi_mix(s, 51u) % 4001u) / 8.0 - 250.0);
    (*p).m4[4].m0.m0 = (short)((long long)(abi_mix(s, 52u) % 65536u) - 32768);
    (*p).m4[4].m0.m1 = (int)((long long)(abi_mix(s, 53u) % 4294967296u) - 2147483648);
    (*p).m4[4].m0.m2 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 54u) ^ ((unsigned _BitInt(129))abi_mix(s, 54u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 54u + 3000u) << 65));
    (*p).m4[4].m0.m3 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 55u) ^ ((unsigned _BitInt(129))abi_mix(s, 55u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 55u + 3000u) << 65));
    (*p).m4[4].m0.m4 = (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 56u) ^ ((unsigned _BitInt(256))abi_mix(s, 56u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 56u + 3000u) << 192));
    (*p).m4[4].m1.m0 = (double)((double)(long long)(abi_mix(s, 57u) % 4001u) / 8.0 - 250.0);
    (*p).m4[4].m1.m1 = (double)((double)(long long)(abi_mix(s, 58u) % 4001u) / 8.0 - 250.0);
    (*p).m4[4].m1.m2 = (double)((double)(long long)(abi_mix(s, 59u) % 4001u) / 8.0 - 250.0);
    (*p).m5_0 = (unsigned)(abi_mix(s, 60u) % 536870912u);
    (*p).m6 = (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 61u) ^ ((unsigned _BitInt(65))abi_mix(s, 61u + 2000u) << 64));
    (*p).m7 = (signed char)((long long)(abi_mix(s, 62u) % 256u) - 128);
}
static int check_R26(const struct R26 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 0u) ^ ((unsigned _BitInt(65))abi_mix(s, 0u + 2000u) << 64));
    bad |= (*p).m0[1] != (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 1u) ^ ((unsigned _BitInt(65))abi_mix(s, 1u + 2000u) << 64));
    bad |= (*p).m0[2] != (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 2u) ^ ((unsigned _BitInt(65))abi_mix(s, 2u + 2000u) << 64));
    bad |= (*p).m0[3] != (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 3u) ^ ((unsigned _BitInt(65))abi_mix(s, 3u + 2000u) << 64));
    bad |= (*p).m0[4] != (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 4u) ^ ((unsigned _BitInt(65))abi_mix(s, 4u + 2000u) << 64));
    bad |= (*p).m1.m0_0 != (_Bool)(abi_mix(s, 5u) & 1u);
    bad |= (*p).m1.m0_1 != (unsigned)(abi_mix(s, 6u) % 536870912u);
    bad |= (*p).m1.m0_2 != (int)((long long)(abi_mix(s, 7u) % 512u) - 256);
    bad |= (*p).m1.m1_0 != (int)((long long)(abi_mix(s, 8u) % 16u) - 8);
    bad |= (*p).m1.m1_1 != (unsigned)(abi_mix(s, 9u) % 131072u);
    bad |= (*p).m1.m1_2 != (unsigned)(abi_mix(s, 10u) % 1073741824u);
    bad |= (*p).m1.m1_3 != (int)((long long)(abi_mix(s, 11u) % 536870912u) - 268435456);
    bad |= (*p).m1.m2 != (double)((double)(long long)(abi_mix(s, 12u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2[0] != (unsigned short)(abi_mix(s, 13u) % 65536u);
    bad |= (*p).m2[1] != (unsigned short)(abi_mix(s, 14u) % 65536u);
    bad |= (*p).m2[2] != (unsigned short)(abi_mix(s, 15u) % 65536u);
    bad |= (*p).m2[3] != (unsigned short)(abi_mix(s, 16u) % 65536u);
    bad |= (*p).m3[0] != (abi_anchor + (abi_mix(s, 17u) % 8u));
    bad |= (*p).m3[1] != (abi_anchor + (abi_mix(s, 18u) % 8u));
    bad |= (*p).m3[2] != (abi_anchor + (abi_mix(s, 19u) % 8u));
    bad |= (*p).m4[0].m0.m0 != (short)((long long)(abi_mix(s, 20u) % 65536u) - 32768);
    bad |= (*p).m4[0].m0.m1 != (int)((long long)(abi_mix(s, 21u) % 4294967296u) - 2147483648);
    bad |= (*p).m4[0].m0.m2 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 22u) ^ ((unsigned _BitInt(129))abi_mix(s, 22u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 22u + 3000u) << 65));
    bad |= (*p).m4[0].m0.m3 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 23u) ^ ((unsigned _BitInt(129))abi_mix(s, 23u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 23u + 3000u) << 65));
    bad |= (*p).m4[0].m0.m4 != (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 24u) ^ ((unsigned _BitInt(256))abi_mix(s, 24u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 24u + 3000u) << 192));
    bad |= (*p).m4[0].m1.m0 != (double)((double)(long long)(abi_mix(s, 25u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[0].m1.m1 != (double)((double)(long long)(abi_mix(s, 26u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[0].m1.m2 != (double)((double)(long long)(abi_mix(s, 27u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[1].m0.m0 != (short)((long long)(abi_mix(s, 28u) % 65536u) - 32768);
    bad |= (*p).m4[1].m0.m1 != (int)((long long)(abi_mix(s, 29u) % 4294967296u) - 2147483648);
    bad |= (*p).m4[1].m0.m2 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 30u) ^ ((unsigned _BitInt(129))abi_mix(s, 30u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 30u + 3000u) << 65));
    bad |= (*p).m4[1].m0.m3 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 31u) ^ ((unsigned _BitInt(129))abi_mix(s, 31u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 31u + 3000u) << 65));
    bad |= (*p).m4[1].m0.m4 != (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 32u) ^ ((unsigned _BitInt(256))abi_mix(s, 32u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 32u + 3000u) << 192));
    bad |= (*p).m4[1].m1.m0 != (double)((double)(long long)(abi_mix(s, 33u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[1].m1.m1 != (double)((double)(long long)(abi_mix(s, 34u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[1].m1.m2 != (double)((double)(long long)(abi_mix(s, 35u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[2].m0.m0 != (short)((long long)(abi_mix(s, 36u) % 65536u) - 32768);
    bad |= (*p).m4[2].m0.m1 != (int)((long long)(abi_mix(s, 37u) % 4294967296u) - 2147483648);
    bad |= (*p).m4[2].m0.m2 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 38u) ^ ((unsigned _BitInt(129))abi_mix(s, 38u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 38u + 3000u) << 65));
    bad |= (*p).m4[2].m0.m3 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 39u) ^ ((unsigned _BitInt(129))abi_mix(s, 39u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 39u + 3000u) << 65));
    bad |= (*p).m4[2].m0.m4 != (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 40u) ^ ((unsigned _BitInt(256))abi_mix(s, 40u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 40u + 3000u) << 192));
    bad |= (*p).m4[2].m1.m0 != (double)((double)(long long)(abi_mix(s, 41u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[2].m1.m1 != (double)((double)(long long)(abi_mix(s, 42u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[2].m1.m2 != (double)((double)(long long)(abi_mix(s, 43u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[3].m0.m0 != (short)((long long)(abi_mix(s, 44u) % 65536u) - 32768);
    bad |= (*p).m4[3].m0.m1 != (int)((long long)(abi_mix(s, 45u) % 4294967296u) - 2147483648);
    bad |= (*p).m4[3].m0.m2 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 46u) ^ ((unsigned _BitInt(129))abi_mix(s, 46u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 46u + 3000u) << 65));
    bad |= (*p).m4[3].m0.m3 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 47u) ^ ((unsigned _BitInt(129))abi_mix(s, 47u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 47u + 3000u) << 65));
    bad |= (*p).m4[3].m0.m4 != (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 48u) ^ ((unsigned _BitInt(256))abi_mix(s, 48u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 48u + 3000u) << 192));
    bad |= (*p).m4[3].m1.m0 != (double)((double)(long long)(abi_mix(s, 49u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[3].m1.m1 != (double)((double)(long long)(abi_mix(s, 50u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[3].m1.m2 != (double)((double)(long long)(abi_mix(s, 51u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[4].m0.m0 != (short)((long long)(abi_mix(s, 52u) % 65536u) - 32768);
    bad |= (*p).m4[4].m0.m1 != (int)((long long)(abi_mix(s, 53u) % 4294967296u) - 2147483648);
    bad |= (*p).m4[4].m0.m2 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 54u) ^ ((unsigned _BitInt(129))abi_mix(s, 54u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 54u + 3000u) << 65));
    bad |= (*p).m4[4].m0.m3 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 55u) ^ ((unsigned _BitInt(129))abi_mix(s, 55u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 55u + 3000u) << 65));
    bad |= (*p).m4[4].m0.m4 != (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 56u) ^ ((unsigned _BitInt(256))abi_mix(s, 56u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 56u + 3000u) << 192));
    bad |= (*p).m4[4].m1.m0 != (double)((double)(long long)(abi_mix(s, 57u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[4].m1.m1 != (double)((double)(long long)(abi_mix(s, 58u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4[4].m1.m2 != (double)((double)(long long)(abi_mix(s, 59u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5_0 != (unsigned)(abi_mix(s, 60u) % 536870912u);
    bad |= (*p).m6 != (_BitInt(65))((unsigned _BitInt(65))abi_mix(s, 61u) ^ ((unsigned _BitInt(65))abi_mix(s, 61u + 2000u) << 64));
    bad |= (*p).m7 != (signed char)((long long)(abi_mix(s, 62u) % 256u) - 128);
    return !bad;
}
static void fill_R27(struct R27 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2] = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3] = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m0[4] = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R27(const struct R27 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2] != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3] != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[4] != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R28(struct R28 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned long long)abi_mix(s, 0u);
    (*p).m0[1] = (unsigned long long)abi_mix(s, 1u);
    (*p).m0[2] = (unsigned long long)abi_mix(s, 2u);
    (*p).m0[3] = (unsigned long long)abi_mix(s, 3u);
    (*p).m0[4] = (unsigned long long)abi_mix(s, 4u);
}
static int check_R28(const struct R28 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m0[1] != (unsigned long long)abi_mix(s, 1u);
    bad |= (*p).m0[2] != (unsigned long long)abi_mix(s, 2u);
    bad |= (*p).m0[3] != (unsigned long long)abi_mix(s, 3u);
    bad |= (*p).m0[4] != (unsigned long long)abi_mix(s, 4u);
    return !bad;
}
static void fill_R29(struct R29 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m1 = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m2 = (long long)abi_mix(s, 2u);
}
static int check_R29(const struct R29 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m1 != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m2 != (long long)abi_mix(s, 2u);
    return !bad;
}
static void fill_R30(union R30 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
}
static int check_R30(const union R30 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    return !bad;
}
static void fill_R31(struct R31 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R31(const struct R31 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R32(struct R32 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R32(const struct R32 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R33(struct R33 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m0 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m1 = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R33(const struct R33 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m0 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m1 != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R34(struct R34 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 0u) ^ ((unsigned _BitInt(256))abi_mix(s, 0u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 0u + 3000u) << 192));
}
static int check_R34(const struct R34 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned _BitInt(256))((unsigned _BitInt(256))abi_mix(s, 0u) ^ ((unsigned _BitInt(256))abi_mix(s, 0u + 2000u) << 64) ^ ((unsigned _BitInt(256))abi_mix(s, 0u + 3000u) << 192));
    return !bad;
}
static void fill_R35(struct R35 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 2097152u) - 1048576);
    (*p).m0_1 = (_Bool)(abi_mix(s, 1u) & 1u);
    (*p).m0_2 = (int)((long long)(abi_mix(s, 2u) % 16777216u) - 8388608);
}
static int check_R35(const struct R35 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 2097152u) - 1048576);
    bad |= (*p).m0_1 != (_Bool)(abi_mix(s, 1u) & 1u);
    bad |= (*p).m0_2 != (int)((long long)(abi_mix(s, 2u) % 16777216u) - 8388608);
    return !bad;
}
static void fill_R36(struct R36 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m0[1] = (unsigned char)(abi_mix(s, 1u) % 256u);
    (*p).m0[2] = (unsigned char)(abi_mix(s, 2u) % 256u);
}
static int check_R36(const struct R36 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m0[1] != (unsigned char)(abi_mix(s, 1u) % 256u);
    bad |= (*p).m0[2] != (unsigned char)(abi_mix(s, 2u) % 256u);
    return !bad;
}
static void fill_R37(struct R37 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 0u) ^ ((unsigned _BitInt(129))abi_mix(s, 0u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 0u + 3000u) << 65));
}
static int check_R37(const struct R37 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned _BitInt(129))((unsigned _BitInt(129))abi_mix(s, 0u) ^ ((unsigned _BitInt(129))abi_mix(s, 0u + 2000u) << 64) ^ ((unsigned _BitInt(129))abi_mix(s, 0u + 3000u) << 65));
    return !bad;
}
static void fill_R38(struct R38 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m1 = (_Bool)(abi_mix(s, 1u) & 1u);
}
static int check_R38(const struct R38 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m1 != (_Bool)(abi_mix(s, 1u) & 1u);
    return !bad;
}
static void fill_R39(struct R39 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
    (*p).m1 = (unsigned long long)abi_mix(s, 1u);
}
static int check_R39(const struct R39 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    bad |= (*p).m1 != (unsigned long long)abi_mix(s, 1u);
    return !bad;
}
static void fill_R40(struct R40 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (int)((long long)(abi_mix(s, 0u) % 134217728u) - 67108864);
}
static int check_R40(const struct R40 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (int)((long long)(abi_mix(s, 0u) % 134217728u) - 67108864);
    return !bad;
}
static void fill_R41(union R41 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    (*p).m0[1] = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    (*p).m0[2] = (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    (*p).m0[3] = (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
}
static int check_R41(const union R41 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (short)((long long)(abi_mix(s, 0u) % 65536u) - 32768);
    bad |= (*p).m0[1] != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    bad |= (*p).m0[2] != (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    bad |= (*p).m0[3] != (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
    return !bad;
}

struct R0 abi_f0(void);
void abi_f1(long long a0, long long a1);
struct R1 abi_f2(float a0, float a1, struct R2 a2, struct R4 a3, struct R5 a4, struct R6 a5, union R12 a6, struct R13 a7, struct R14 a8, _BitInt(128) a9, signed char a10, double a11, ...);
struct R18 abi_f3(unsigned long long a0, ...);
_Bool abi_f4(union R21 a0, struct R26 a1);
struct R27 abi_f5(struct R28 a0);
struct R29 abi_f6(union R30 a0, _BitInt(65) a1, struct R31 a2, struct R33 a3);
void abi_f7(unsigned char a0, unsigned __int128 a1, struct R34 a2, struct R35 a3, struct R36 a4, struct R37 a5, unsigned long long a6, struct R38 a7, ...);
#endif
