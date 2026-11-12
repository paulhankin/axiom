/* abi_gen_103.h - written by gen_c_abi.py 103 8: the types of the
   tests in abi_gen_103.c and abi_gen_103_callee.c, and the functions that fill
   and check them. */
#ifndef ABI_GEN_103_H
#define ABI_GEN_103_H
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
    _Bool m0;
    unsigned m1_0 : 15;
    unsigned char m2;
    char * m3[4];
};
struct R1 {
    struct R0 m0;
    unsigned short m1[5];
};
struct R2 {
    int m0;
    int m1_0 : 27;
    _Bool m1_1 : 1;
    unsigned m1_2 : 30;
};
struct R3 {
    int m0;
    float m1[1];
    struct R2 m2;
};
struct R4 {
    struct R3 m0;
    int m1;
};
union R5 {
    int m0[1];
};
struct R6 {
    float m0;
    float m1;
};
struct R7 {
    struct R6 m0;
};
struct R8 {
    double m0;
    double m1;
};
struct R9 {
    _Bool m0;
    short m1;
    int m2;
    unsigned m3_0 : 9;
    unsigned : 0;
    int m3_1 : 10;
    int m4;
    int m5_0 : 11;
    _Bool m5_1 : 1;
};
struct R10 {
    double m0;
    double m1[1];
    double m2;
};
struct R11 {
    _Bool m0_0 : 1;
    unsigned m1_0 : 5;
    int m1_1 : 10;
    _Bool m1_2 : 1;
};
struct R12 {
    struct R11 m0;
};
union R13 {
    unsigned short m0[2];
};
union R14 {
    long long m0[1];
    struct R12 m1;
    signed char m2;
    union R13 m3;
    unsigned char m4;
    long long m5;
};
struct R15 {
    unsigned short m0;
    long long m1;
};
struct R16 {
    float m0;
    struct R15 m1;
    unsigned short m2[5];
    int m3;
    float m4;
    unsigned long long m5;
    unsigned short m6;
    unsigned char m7;
};
union R17 {
    double m0[2];
    struct R16 m1;
};
struct R18 {
    unsigned m0;
    unsigned m1_0 : 3;
};
struct R19 {
    float m0;
    float m1;
    float m2;
    float m3;
    float m4;
};
struct R20 {
    unsigned m0;
};
struct R21 {
    unsigned long long m0;
    struct R20 m1;
};
struct R22 {
    char * m0;
};
struct R23 {
    double m0;
    double m1;
};
struct R24 {
    struct R23 m0;
};
struct R25 {
    double m0;
    double m1;
};
struct R26 {
    struct R25 m0;
};
struct R27 {
    double m0[4];
};
struct R28 {
    double m0;
};
struct R29 {
    float m0;
};
struct R30 {
    float m0;
    struct R29 m1;
    float m2;
};
struct R31 {
    signed char m0[3];
};
struct R32 {
    struct R31 m0;
    int m1_0 : 8;
    unsigned m1_1 : 16;
    int m1_2 : 22;
    unsigned : 0;
    int m2_0 : 29;
    float m3[5];
    unsigned m4_0 : 18;
    _Bool m4_1 : 1;
    unsigned m4_2 : 20;
};
union R33 {
    unsigned char m0;
    unsigned short m1;
    unsigned char m2;
};
struct R34 {
    int m0;
    double m1;
    short m2;
};
struct R35 {
    int m0;
};
struct R36 {
    char * m0;
    struct R34 m1;
    struct R35 m2;
    int m3_0 : 8;
    unsigned m3_1 : 9;
    unsigned m3_2 : 18;
    _Bool m3_3 : 1;
};
struct R37 {
    unsigned m0;
    double m1;
    long long m2;
    unsigned m3_0 : 22;
    _Bool m4_0 : 1;
    unsigned m4_1 : 16;
    unsigned short m5;
    unsigned char m6;
    unsigned short m7[3];
};
union R38 {
    union R33 m0;
    struct R36 m1;
    struct R37 m2;
    unsigned long long m3;
    unsigned char m4;
};
union R39 {
    unsigned char m0;
    char * m1;
    int m2;
    short m3;
    int m4;
};
struct R40 {
    double m0;
    double m1;
};
struct R41 {
    int m0;
    unsigned m1_0 : 16;
    int m1_1 : 8;
    unsigned : 3;
    int m1_2 : 25;
    int m2;
    long long m3[1];
};
struct R42 {
    unsigned m0;
};
struct R43 {
    struct R41 m0;
    struct R42 m1;
};
struct R44 {
    float m0;
    float m1;
    float m2;
};
struct R45 {
    float m0;
    struct R44 m1;
    float m2;
};
struct R46 {
    double m0;
};
struct R47 {
    unsigned m0;
};

static void fill_R0(struct R0 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m1_0 = (unsigned)(abi_mix(s, 1u) % 32768u);
    (*p).m2 = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p).m3[0] = (abi_anchor + (abi_mix(s, 3u) % 8u));
    (*p).m3[1] = (abi_anchor + (abi_mix(s, 4u) % 8u));
    (*p).m3[2] = (abi_anchor + (abi_mix(s, 5u) % 8u));
    (*p).m3[3] = (abi_anchor + (abi_mix(s, 6u) % 8u));
}
static int check_R0(const struct R0 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 1u) % 32768u);
    bad |= (*p).m2 != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m3[0] != (abi_anchor + (abi_mix(s, 3u) % 8u));
    bad |= (*p).m3[1] != (abi_anchor + (abi_mix(s, 4u) % 8u));
    bad |= (*p).m3[2] != (abi_anchor + (abi_mix(s, 5u) % 8u));
    bad |= (*p).m3[3] != (abi_anchor + (abi_mix(s, 6u) % 8u));
    return !bad;
}
static void fill_R1(struct R1 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m0.m1_0 = (unsigned)(abi_mix(s, 1u) % 32768u);
    (*p).m0.m2 = (unsigned char)(abi_mix(s, 2u) % 256u);
    (*p).m0.m3[0] = (abi_anchor + (abi_mix(s, 3u) % 8u));
    (*p).m0.m3[1] = (abi_anchor + (abi_mix(s, 4u) % 8u));
    (*p).m0.m3[2] = (abi_anchor + (abi_mix(s, 5u) % 8u));
    (*p).m0.m3[3] = (abi_anchor + (abi_mix(s, 6u) % 8u));
    (*p).m1[0] = (unsigned short)(abi_mix(s, 7u) % 65536u);
    (*p).m1[1] = (unsigned short)(abi_mix(s, 8u) % 65536u);
    (*p).m1[2] = (unsigned short)(abi_mix(s, 9u) % 65536u);
    (*p).m1[3] = (unsigned short)(abi_mix(s, 10u) % 65536u);
    (*p).m1[4] = (unsigned short)(abi_mix(s, 11u) % 65536u);
}
static int check_R1(const struct R1 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m0.m1_0 != (unsigned)(abi_mix(s, 1u) % 32768u);
    bad |= (*p).m0.m2 != (unsigned char)(abi_mix(s, 2u) % 256u);
    bad |= (*p).m0.m3[0] != (abi_anchor + (abi_mix(s, 3u) % 8u));
    bad |= (*p).m0.m3[1] != (abi_anchor + (abi_mix(s, 4u) % 8u));
    bad |= (*p).m0.m3[2] != (abi_anchor + (abi_mix(s, 5u) % 8u));
    bad |= (*p).m0.m3[3] != (abi_anchor + (abi_mix(s, 6u) % 8u));
    bad |= (*p).m1[0] != (unsigned short)(abi_mix(s, 7u) % 65536u);
    bad |= (*p).m1[1] != (unsigned short)(abi_mix(s, 8u) % 65536u);
    bad |= (*p).m1[2] != (unsigned short)(abi_mix(s, 9u) % 65536u);
    bad |= (*p).m1[3] != (unsigned short)(abi_mix(s, 10u) % 65536u);
    bad |= (*p).m1[4] != (unsigned short)(abi_mix(s, 11u) % 65536u);
    return !bad;
}
static void fill_R2(struct R2 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p).m1_0 = (int)((long long)(abi_mix(s, 1u) % 134217728u) - 67108864);
    (*p).m1_1 = (_Bool)(abi_mix(s, 2u) & 1u);
    (*p).m1_2 = (unsigned)(abi_mix(s, 3u) % 1073741824u);
}
static int check_R2(const struct R2 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p).m1_0 != (int)((long long)(abi_mix(s, 1u) % 134217728u) - 67108864);
    bad |= (*p).m1_1 != (_Bool)(abi_mix(s, 2u) & 1u);
    bad |= (*p).m1_2 != (unsigned)(abi_mix(s, 3u) % 1073741824u);
    return !bad;
}
static void fill_R3(struct R3 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p).m1[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2.m0 = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p).m2.m1_0 = (int)((long long)(abi_mix(s, 3u) % 134217728u) - 67108864);
    (*p).m2.m1_1 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m2.m1_2 = (unsigned)(abi_mix(s, 5u) % 1073741824u);
}
static int check_R3(const struct R3 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p).m1[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2.m0 != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p).m2.m1_0 != (int)((long long)(abi_mix(s, 3u) % 134217728u) - 67108864);
    bad |= (*p).m2.m1_1 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m2.m1_2 != (unsigned)(abi_mix(s, 5u) % 1073741824u);
    return !bad;
}
static void fill_R4(struct R4 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p).m0.m1[0] = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m2.m0 = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p).m0.m2.m1_0 = (int)((long long)(abi_mix(s, 3u) % 134217728u) - 67108864);
    (*p).m0.m2.m1_1 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m0.m2.m1_2 = (unsigned)(abi_mix(s, 5u) % 1073741824u);
    (*p).m1 = (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
}
static int check_R4(const struct R4 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p).m0.m1[0] != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m2.m0 != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p).m0.m2.m1_0 != (int)((long long)(abi_mix(s, 3u) % 134217728u) - 67108864);
    bad |= (*p).m0.m2.m1_1 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m0.m2.m1_2 != (unsigned)(abi_mix(s, 5u) % 1073741824u);
    bad |= (*p).m1 != (int)((long long)(abi_mix(s, 6u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R5(union R5 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
}
static int check_R5(const union R5 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R6(struct R6 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R6(const struct R6 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R7(struct R7 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R7(const struct R7 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R8(struct R8 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R8(const struct R8 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R9(struct R9 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m1 = (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    (*p).m2 = (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    (*p).m3_0 = (unsigned)(abi_mix(s, 3u) % 512u);
    (*p).m3_1 = (int)((long long)(abi_mix(s, 4u) % 1024u) - 512);
    (*p).m4 = (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    (*p).m5_0 = (int)((long long)(abi_mix(s, 6u) % 2048u) - 1024);
    (*p).m5_1 = (_Bool)(abi_mix(s, 7u) & 1u);
}
static int check_R9(const struct R9 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m1 != (short)((long long)(abi_mix(s, 1u) % 65536u) - 32768);
    bad |= (*p).m2 != (int)((long long)(abi_mix(s, 2u) % 4294967296u) - 2147483648);
    bad |= (*p).m3_0 != (unsigned)(abi_mix(s, 3u) % 512u);
    bad |= (*p).m3_1 != (int)((long long)(abi_mix(s, 4u) % 1024u) - 512);
    bad |= (*p).m4 != (int)((long long)(abi_mix(s, 5u) % 4294967296u) - 2147483648);
    bad |= (*p).m5_0 != (int)((long long)(abi_mix(s, 6u) % 2048u) - 1024);
    bad |= (*p).m5_1 != (_Bool)(abi_mix(s, 7u) & 1u);
    return !bad;
}
static void fill_R10(struct R10 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1[0] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R10(const struct R10 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1[0] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R11(struct R11 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0_0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m1_0 = (unsigned)(abi_mix(s, 1u) % 32u);
    (*p).m1_1 = (int)((long long)(abi_mix(s, 2u) % 1024u) - 512);
    (*p).m1_2 = (_Bool)(abi_mix(s, 3u) & 1u);
}
static int check_R11(const struct R11 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0_0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 1u) % 32u);
    bad |= (*p).m1_1 != (int)((long long)(abi_mix(s, 2u) % 1024u) - 512);
    bad |= (*p).m1_2 != (_Bool)(abi_mix(s, 3u) & 1u);
    return !bad;
}
static void fill_R12(struct R12 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0_0 = (_Bool)(abi_mix(s, 0u) & 1u);
    (*p).m0.m1_0 = (unsigned)(abi_mix(s, 1u) % 32u);
    (*p).m0.m1_1 = (int)((long long)(abi_mix(s, 2u) % 1024u) - 512);
    (*p).m0.m1_2 = (_Bool)(abi_mix(s, 3u) & 1u);
}
static int check_R12(const struct R12 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0_0 != (_Bool)(abi_mix(s, 0u) & 1u);
    bad |= (*p).m0.m1_0 != (unsigned)(abi_mix(s, 1u) % 32u);
    bad |= (*p).m0.m1_1 != (int)((long long)(abi_mix(s, 2u) % 1024u) - 512);
    bad |= (*p).m0.m1_2 != (_Bool)(abi_mix(s, 3u) & 1u);
    return !bad;
}
static void fill_R13(union R13 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m0[1] = (unsigned short)(abi_mix(s, 1u) % 65536u);
}
static int check_R13(const union R13 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m0[1] != (unsigned short)(abi_mix(s, 1u) % 65536u);
    return !bad;
}
static void fill_R14(union R14 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (long long)abi_mix(s, 0u);
}
static int check_R14(const union R14 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (long long)abi_mix(s, 0u);
    return !bad;
}
static void fill_R15(struct R15 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned short)(abi_mix(s, 0u) % 65536u);
    (*p).m1 = (long long)abi_mix(s, 1u);
}
static int check_R15(const struct R15 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned short)(abi_mix(s, 0u) % 65536u);
    bad |= (*p).m1 != (long long)abi_mix(s, 1u);
    return !bad;
}
static void fill_R16(struct R16 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (unsigned short)(abi_mix(s, 1u) % 65536u);
    (*p).m1.m1 = (long long)abi_mix(s, 2u);
    (*p).m2[0] = (unsigned short)(abi_mix(s, 3u) % 65536u);
    (*p).m2[1] = (unsigned short)(abi_mix(s, 4u) % 65536u);
    (*p).m2[2] = (unsigned short)(abi_mix(s, 5u) % 65536u);
    (*p).m2[3] = (unsigned short)(abi_mix(s, 6u) % 65536u);
    (*p).m2[4] = (unsigned short)(abi_mix(s, 7u) % 65536u);
    (*p).m3 = (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    (*p).m4 = (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    (*p).m5 = (unsigned long long)abi_mix(s, 10u);
    (*p).m6 = (unsigned short)(abi_mix(s, 11u) % 65536u);
    (*p).m7 = (unsigned char)(abi_mix(s, 12u) % 256u);
}
static int check_R16(const struct R16 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (unsigned short)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m1.m1 != (long long)abi_mix(s, 2u);
    bad |= (*p).m2[0] != (unsigned short)(abi_mix(s, 3u) % 65536u);
    bad |= (*p).m2[1] != (unsigned short)(abi_mix(s, 4u) % 65536u);
    bad |= (*p).m2[2] != (unsigned short)(abi_mix(s, 5u) % 65536u);
    bad |= (*p).m2[3] != (unsigned short)(abi_mix(s, 6u) % 65536u);
    bad |= (*p).m2[4] != (unsigned short)(abi_mix(s, 7u) % 65536u);
    bad |= (*p).m3 != (int)((long long)(abi_mix(s, 8u) % 4294967296u) - 2147483648);
    bad |= (*p).m4 != (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m5 != (unsigned long long)abi_mix(s, 10u);
    bad |= (*p).m6 != (unsigned short)(abi_mix(s, 11u) % 65536u);
    bad |= (*p).m7 != (unsigned char)(abi_mix(s, 12u) % 256u);
    return !bad;
}
static void fill_R17(union R17 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R17(const union R17 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R18(struct R18 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
    (*p).m1_0 = (unsigned)(abi_mix(s, 1u) % 8u);
}
static int check_R18(const struct R18 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 1u) % 8u);
    return !bad;
}
static void fill_R19(struct R19 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m3 = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m4 = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R19(const struct R19 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3 != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4 != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R20(struct R20 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
}
static int check_R20(const struct R20 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    return !bad;
}
static void fill_R21(struct R21 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned long long)abi_mix(s, 0u);
    (*p).m1.m0 = (unsigned)(abi_mix(s, 1u) % 4294967296u);
}
static int check_R21(const struct R21 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned long long)abi_mix(s, 0u);
    bad |= (*p).m1.m0 != (unsigned)(abi_mix(s, 1u) % 4294967296u);
    return !bad;
}
static void fill_R22(struct R22 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
}
static int check_R22(const struct R22 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    return !bad;
}
static void fill_R23(struct R23 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R23(const struct R23 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R24(struct R24 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R24(const struct R24 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R25(struct R25 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R25(const struct R25 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R26(struct R26 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0.m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R26(const struct R26 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0.m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R27(struct R27 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m0[1] = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m0[2] = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m0[3] = (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
}
static int check_R27(const struct R27 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[1] != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[2] != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m0[3] != (double)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R28(struct R28 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R28(const struct R28 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
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
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R30(const struct R30 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R31(struct R31 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0[0] = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0[1] = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m0[2] = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
}
static int check_R31(const struct R31 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0[0] != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0[1] != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m0[2] != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    return !bad;
}
static void fill_R32(struct R32 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0[0] = (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    (*p).m0.m0[1] = (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    (*p).m0.m0[2] = (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m1_0 = (int)((long long)(abi_mix(s, 3u) % 256u) - 128);
    (*p).m1_1 = (unsigned)(abi_mix(s, 4u) % 65536u);
    (*p).m1_2 = (int)((long long)(abi_mix(s, 5u) % 4194304u) - 2097152);
    (*p).m2_0 = (int)((long long)(abi_mix(s, 6u) % 536870912u) - 268435456);
    (*p).m3[0] = (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    (*p).m3[1] = (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    (*p).m3[2] = (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    (*p).m3[3] = (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    (*p).m3[4] = (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    (*p).m4_0 = (unsigned)(abi_mix(s, 12u) % 262144u);
    (*p).m4_1 = (_Bool)(abi_mix(s, 13u) & 1u);
    (*p).m4_2 = (unsigned)(abi_mix(s, 14u) % 1048576u);
}
static int check_R32(const struct R32 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0[0] != (signed char)((long long)(abi_mix(s, 0u) % 256u) - 128);
    bad |= (*p).m0.m0[1] != (signed char)((long long)(abi_mix(s, 1u) % 256u) - 128);
    bad |= (*p).m0.m0[2] != (signed char)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m1_0 != (int)((long long)(abi_mix(s, 3u) % 256u) - 128);
    bad |= (*p).m1_1 != (unsigned)(abi_mix(s, 4u) % 65536u);
    bad |= (*p).m1_2 != (int)((long long)(abi_mix(s, 5u) % 4194304u) - 2097152);
    bad |= (*p).m2_0 != (int)((long long)(abi_mix(s, 6u) % 536870912u) - 268435456);
    bad |= (*p).m3[0] != (float)((double)(long long)(abi_mix(s, 7u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[1] != (float)((double)(long long)(abi_mix(s, 8u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[2] != (float)((double)(long long)(abi_mix(s, 9u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[3] != (float)((double)(long long)(abi_mix(s, 10u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m3[4] != (float)((double)(long long)(abi_mix(s, 11u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m4_0 != (unsigned)(abi_mix(s, 12u) % 262144u);
    bad |= (*p).m4_1 != (_Bool)(abi_mix(s, 13u) & 1u);
    bad |= (*p).m4_2 != (unsigned)(abi_mix(s, 14u) % 1048576u);
    return !bad;
}
static void fill_R33(union R33 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
}
static int check_R33(const union R33 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    return !bad;
}
static void fill_R34(struct R34 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
}
static int check_R34(const struct R34 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (short)((long long)(abi_mix(s, 2u) % 65536u) - 32768);
    return !bad;
}
static void fill_R35(struct R35 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
}
static int check_R35(const struct R35 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    return !bad;
}
static void fill_R36(struct R36 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (abi_anchor + (abi_mix(s, 0u) % 8u));
    (*p).m1.m0 = (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    (*p).m1.m1 = (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2 = (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
    (*p).m2.m0 = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p).m3_0 = (int)((long long)(abi_mix(s, 5u) % 256u) - 128);
    (*p).m3_1 = (unsigned)(abi_mix(s, 6u) % 512u);
    (*p).m3_2 = (unsigned)(abi_mix(s, 7u) % 262144u);
    (*p).m3_3 = (_Bool)(abi_mix(s, 8u) & 1u);
}
static int check_R36(const struct R36 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (abi_anchor + (abi_mix(s, 0u) % 8u));
    bad |= (*p).m1.m0 != (int)((long long)(abi_mix(s, 1u) % 4294967296u) - 2147483648);
    bad |= (*p).m1.m1 != (double)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2 != (short)((long long)(abi_mix(s, 3u) % 65536u) - 32768);
    bad |= (*p).m2.m0 != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p).m3_0 != (int)((long long)(abi_mix(s, 5u) % 256u) - 128);
    bad |= (*p).m3_1 != (unsigned)(abi_mix(s, 6u) % 512u);
    bad |= (*p).m3_2 != (unsigned)(abi_mix(s, 7u) % 262144u);
    bad |= (*p).m3_3 != (_Bool)(abi_mix(s, 8u) & 1u);
    return !bad;
}
static void fill_R37(struct R37 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (long long)abi_mix(s, 2u);
    (*p).m3_0 = (unsigned)(abi_mix(s, 3u) % 4194304u);
    (*p).m4_0 = (_Bool)(abi_mix(s, 4u) & 1u);
    (*p).m4_1 = (unsigned)(abi_mix(s, 5u) % 65536u);
    (*p).m5 = (unsigned short)(abi_mix(s, 6u) % 65536u);
    (*p).m6 = (unsigned char)(abi_mix(s, 7u) % 256u);
    (*p).m7[0] = (unsigned short)(abi_mix(s, 8u) % 65536u);
    (*p).m7[1] = (unsigned short)(abi_mix(s, 9u) % 65536u);
    (*p).m7[2] = (unsigned short)(abi_mix(s, 10u) % 65536u);
}
static int check_R37(const struct R37 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (long long)abi_mix(s, 2u);
    bad |= (*p).m3_0 != (unsigned)(abi_mix(s, 3u) % 4194304u);
    bad |= (*p).m4_0 != (_Bool)(abi_mix(s, 4u) & 1u);
    bad |= (*p).m4_1 != (unsigned)(abi_mix(s, 5u) % 65536u);
    bad |= (*p).m5 != (unsigned short)(abi_mix(s, 6u) % 65536u);
    bad |= (*p).m6 != (unsigned char)(abi_mix(s, 7u) % 256u);
    bad |= (*p).m7[0] != (unsigned short)(abi_mix(s, 8u) % 65536u);
    bad |= (*p).m7[1] != (unsigned short)(abi_mix(s, 9u) % 65536u);
    bad |= (*p).m7[2] != (unsigned short)(abi_mix(s, 10u) % 65536u);
    return !bad;
}
static void fill_R38(union R38 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
}
static int check_R38(const union R38 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    return !bad;
}
static void fill_R39(union R39 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned char)(abi_mix(s, 0u) % 256u);
}
static int check_R39(const union R39 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned char)(abi_mix(s, 0u) % 256u);
    return !bad;
}
static void fill_R40(struct R40 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
}
static int check_R40(const struct R40 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (double)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R41(struct R41 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p).m1_0 = (unsigned)(abi_mix(s, 1u) % 65536u);
    (*p).m1_1 = (int)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m1_2 = (int)((long long)(abi_mix(s, 3u) % 33554432u) - 16777216);
    (*p).m2 = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p).m3[0] = (long long)abi_mix(s, 5u);
}
static int check_R41(const struct R41 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p).m1_0 != (unsigned)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m1_1 != (int)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m1_2 != (int)((long long)(abi_mix(s, 3u) % 33554432u) - 16777216);
    bad |= (*p).m2 != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p).m3[0] != (long long)abi_mix(s, 5u);
    return !bad;
}
static void fill_R42(struct R42 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
}
static int check_R42(const struct R42 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    return !bad;
}
static void fill_R43(struct R43 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0.m0 = (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    (*p).m0.m1_0 = (unsigned)(abi_mix(s, 1u) % 65536u);
    (*p).m0.m1_1 = (int)((long long)(abi_mix(s, 2u) % 256u) - 128);
    (*p).m0.m1_2 = (int)((long long)(abi_mix(s, 3u) % 33554432u) - 16777216);
    (*p).m0.m2 = (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    (*p).m0.m3[0] = (long long)abi_mix(s, 5u);
    (*p).m1.m0 = (unsigned)(abi_mix(s, 6u) % 4294967296u);
}
static int check_R43(const struct R43 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0.m0 != (int)((long long)(abi_mix(s, 0u) % 4294967296u) - 2147483648);
    bad |= (*p).m0.m1_0 != (unsigned)(abi_mix(s, 1u) % 65536u);
    bad |= (*p).m0.m1_1 != (int)((long long)(abi_mix(s, 2u) % 256u) - 128);
    bad |= (*p).m0.m1_2 != (int)((long long)(abi_mix(s, 3u) % 33554432u) - 16777216);
    bad |= (*p).m0.m2 != (int)((long long)(abi_mix(s, 4u) % 4294967296u) - 2147483648);
    bad |= (*p).m0.m3[0] != (long long)abi_mix(s, 5u);
    bad |= (*p).m1.m0 != (unsigned)(abi_mix(s, 6u) % 4294967296u);
    return !bad;
}
static void fill_R44(struct R44 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
}
static int check_R44(const struct R44 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R45(struct R45 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m0 = (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m1 = (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    (*p).m1.m2 = (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    (*p).m2 = (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
}
static int check_R45(const struct R45 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (float)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m0 != (float)((double)(long long)(abi_mix(s, 1u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m1 != (float)((double)(long long)(abi_mix(s, 2u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m1.m2 != (float)((double)(long long)(abi_mix(s, 3u) % 4001u) / 8.0 - 250.0);
    bad |= (*p).m2 != (float)((double)(long long)(abi_mix(s, 4u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R46(struct R46 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
}
static int check_R46(const struct R46 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (double)((double)(long long)(abi_mix(s, 0u) % 4001u) / 8.0 - 250.0);
    return !bad;
}
static void fill_R47(struct R47 *p, unsigned s) {
    abi_zero(p, sizeof *p);
    (*p).m0 = (unsigned)(abi_mix(s, 0u) % 4294967296u);
}
static int check_R47(const struct R47 *p, unsigned s) {
    int bad = 0;
    bad |= (*p).m0 != (unsigned)(abi_mix(s, 0u) % 4294967296u);
    return !bad;
}

struct R1 abi_f0(unsigned short a0, struct R4 a1, union R5 a2);
struct R7 abi_f1(struct R8 a0, float a1, ...);
union R17 abi_f2(struct R18 a0);
struct R19 abi_f3(int a0, struct R21 a1, unsigned long long a2, struct R22 a3, ...);
struct R27 abi_f4(short a0, float a1, struct R28 a2, int a3, struct R30 a4);
void abi_f5(float a0, int a1, double a2, int a3, double a4, double a5, long long a6, long long a7, struct R32 a8);
float abi_f6(union R38 a0, union R39 a1, struct R40 a2, ...);
struct R43 abi_f7(struct R45 a0, short a1, struct R46 a2, struct R47 a3);
#endif
