/* abi_gen_103.c - written by gen_c_abi.py 103 8: calls of the
   functions of abi_gen_103_callee.c, directly, through pointers and with
   variadic arguments; prints the failures and the calls made. */
/* units: abi/abi_gen_103_callee.c */
#include "host.h"
#include "abi/abi_gen_103.h"

int abi_result(void);
int abi_first(void);
unsigned long long abi_layout(int k);

static struct R1 (*volatile abi_p0)(unsigned short a0, struct R4 a1, union R5 a2) = abi_f0;
static struct R7 (*volatile abi_p1)(struct R8 a0, float a1, ...) = abi_f1;
static union R17 (*volatile abi_p2)(struct R18 a0) = abi_f2;
static struct R19 (*volatile abi_p3)(int a0, struct R21 a1, unsigned long long a2, struct R22 a3, ...) = abi_f3;
static struct R27 (*volatile abi_p4)(short a0, float a1, struct R28 a2, int a3, struct R30 a4) = abi_f4;
static void (*volatile abi_p5)(float a0, int a1, double a2, int a3, double a4, double a5, long long a6, long long a7, struct R32 a8) = abi_f5;
static float (*volatile abi_p6)(union R38 a0, union R39 a1, struct R40 a2, ...) = abi_f6;
static struct R43 (*volatile abi_p7)(struct R45 a0, short a1, struct R46 a2, struct R47 a3) = abi_f7;

static int abi_t0(void) {
    unsigned short a0 = (unsigned short)(abi_mix(0u, 0u) % 65536u);
    struct R4 a1; fill_R4(&a1, 1u);
    union R5 a2; fill_R5(&a2, 2u);
    int bad = 0;
    {
        struct R1 r = abi_f0(a0, a1, a2);
        if (!(check_R1(&r, 99u))) bad = 1;
    }
    {
        struct R1 r = abi_p0(a0, a1, a2);
        if (!(check_R1(&r, 99u))) bad = 1;
    }
    if (bad) abi_fail(0, 99);
    return 2;
}

static int abi_t1(void) {
    struct R8 a0; fill_R8(&a0, 100u);
    float a1 = (float)((double)(long long)(abi_mix(101u, 0u) % 4001u) / 8.0 - 250.0);
    struct R9 v0; fill_R9(&v0, 150u);
    struct R10 v1; fill_R10(&v1, 151u);
    union R14 v2; fill_R14(&v2, 152u);
    int bad = 0;
    {
        struct R7 r = abi_f1(a0, a1, v0, v1, v2);
        if (!(check_R7(&r, 199u))) bad = 1;
    }
    {
        struct R7 r = abi_p1(a0, a1, v0, v1, v2);
        if (!(check_R7(&r, 199u))) bad = 1;
    }
    if (bad) abi_fail(1, 99);
    return 2;
}

static int abi_t2(void) {
    struct R18 a0; fill_R18(&a0, 200u);
    int bad = 0;
    {
        union R17 r = abi_f2(a0);
        if (!(check_R17(&r, 299u))) bad = 1;
    }
    {
        union R17 r = abi_p2(a0);
        if (!(check_R17(&r, 299u))) bad = 1;
    }
    if (bad) abi_fail(2, 99);
    return 2;
}

static int abi_t3(void) {
    int a0 = (int)((long long)(abi_mix(300u, 0u) % 4294967296u) - 2147483648);
    struct R21 a1; fill_R21(&a1, 301u);
    unsigned long long a2 = (unsigned long long)abi_mix(302u, 0u);
    struct R22 a3; fill_R22(&a3, 303u);
    struct R24 v0; fill_R24(&v0, 350u);
    struct R26 v1; fill_R26(&v1, 351u);
    int bad = 0;
    {
        struct R19 r = abi_f3(a0, a1, a2, a3, v0, v1);
        if (!(check_R19(&r, 399u))) bad = 1;
    }
    {
        struct R19 r = abi_p3(a0, a1, a2, a3, v0, v1);
        if (!(check_R19(&r, 399u))) bad = 1;
    }
    if (bad) abi_fail(3, 99);
    return 2;
}

static int abi_t4(void) {
    short a0 = (short)((long long)(abi_mix(400u, 0u) % 65536u) - 32768);
    float a1 = (float)((double)(long long)(abi_mix(401u, 0u) % 4001u) / 8.0 - 250.0);
    struct R28 a2; fill_R28(&a2, 402u);
    int a3 = (int)((long long)(abi_mix(403u, 0u) % 4294967296u) - 2147483648);
    struct R30 a4; fill_R30(&a4, 404u);
    int bad = 0;
    {
        struct R27 r = abi_f4(a0, a1, a2, a3, a4);
        if (!(check_R27(&r, 499u))) bad = 1;
    }
    {
        struct R27 r = abi_p4(a0, a1, a2, a3, a4);
        if (!(check_R27(&r, 499u))) bad = 1;
    }
    if (bad) abi_fail(4, 99);
    return 2;
}

static int abi_t5(void) {
    float a0 = (float)((double)(long long)(abi_mix(500u, 0u) % 4001u) / 8.0 - 250.0);
    int a1 = (int)((long long)(abi_mix(501u, 0u) % 4294967296u) - 2147483648);
    double a2 = (double)((double)(long long)(abi_mix(502u, 0u) % 4001u) / 8.0 - 250.0);
    int a3 = (int)((long long)(abi_mix(503u, 0u) % 4294967296u) - 2147483648);
    double a4 = (double)((double)(long long)(abi_mix(504u, 0u) % 4001u) / 8.0 - 250.0);
    double a5 = (double)((double)(long long)(abi_mix(505u, 0u) % 4001u) / 8.0 - 250.0);
    long long a6 = (long long)abi_mix(506u, 0u);
    long long a7 = (long long)abi_mix(507u, 0u);
    struct R32 a8; fill_R32(&a8, 508u);
    int bad = 0;
    abi_f5(a0, a1, a2, a3, a4, a5, a6, a7, a8);
    abi_p5(a0, a1, a2, a3, a4, a5, a6, a7, a8);
    if (bad) abi_fail(5, 99);
    return 2;
}

static int abi_t6(void) {
    union R38 a0; fill_R38(&a0, 600u);
    union R39 a1; fill_R39(&a1, 601u);
    struct R40 a2; fill_R40(&a2, 602u);
    int v0 = (int)((long long)(abi_mix(650u, 0u) % 4294967296u) - 2147483648);
    int bad = 0;
    {
        float r = abi_f6(a0, a1, a2, v0);
        if (!(r == (float)((double)(long long)(abi_mix(699u, 0u) % 4001u) / 8.0 - 250.0))) bad = 1;
    }
    {
        float r = abi_p6(a0, a1, a2, v0);
        if (!(r == (float)((double)(long long)(abi_mix(699u, 0u) % 4001u) / 8.0 - 250.0))) bad = 1;
    }
    if (bad) abi_fail(6, 99);
    return 2;
}

static int abi_t7(void) {
    struct R45 a0; fill_R45(&a0, 700u);
    short a1 = (short)((long long)(abi_mix(701u, 0u) % 65536u) - 32768);
    struct R46 a2; fill_R46(&a2, 702u);
    struct R47 a3; fill_R47(&a3, 703u);
    int bad = 0;
    {
        struct R43 r = abi_f7(a0, a1, a2, a3);
        if (!(check_R43(&r, 799u))) bad = 1;
    }
    {
        struct R43 r = abi_p7(a0, a1, a2, a3);
        if (!(check_R43(&r, 799u))) bad = 1;
    }
    if (bad) abi_fail(7, 99);
    return 2;
}

int main(void) {
    int calls = 0;
    if (abi_layout(0) != sizeof(struct R0) * 1000 + _Alignof(struct R0)) abi_fail(-1, 0);
    if (abi_layout(1) != sizeof(struct R1) * 1000 + _Alignof(struct R1)) abi_fail(-1, 1);
    if (abi_layout(2) != sizeof(struct R2) * 1000 + _Alignof(struct R2)) abi_fail(-1, 2);
    if (abi_layout(3) != sizeof(struct R3) * 1000 + _Alignof(struct R3)) abi_fail(-1, 3);
    if (abi_layout(4) != sizeof(struct R4) * 1000 + _Alignof(struct R4)) abi_fail(-1, 4);
    if (abi_layout(5) != sizeof(union R5) * 1000 + _Alignof(union R5)) abi_fail(-1, 5);
    if (abi_layout(6) != sizeof(struct R6) * 1000 + _Alignof(struct R6)) abi_fail(-1, 6);
    if (abi_layout(7) != sizeof(struct R7) * 1000 + _Alignof(struct R7)) abi_fail(-1, 7);
    if (abi_layout(8) != sizeof(struct R8) * 1000 + _Alignof(struct R8)) abi_fail(-1, 8);
    if (abi_layout(9) != sizeof(struct R9) * 1000 + _Alignof(struct R9)) abi_fail(-1, 9);
    if (abi_layout(10) != sizeof(struct R10) * 1000 + _Alignof(struct R10)) abi_fail(-1, 10);
    if (abi_layout(11) != sizeof(struct R11) * 1000 + _Alignof(struct R11)) abi_fail(-1, 11);
    if (abi_layout(12) != sizeof(struct R12) * 1000 + _Alignof(struct R12)) abi_fail(-1, 12);
    if (abi_layout(13) != sizeof(union R13) * 1000 + _Alignof(union R13)) abi_fail(-1, 13);
    if (abi_layout(14) != sizeof(union R14) * 1000 + _Alignof(union R14)) abi_fail(-1, 14);
    if (abi_layout(15) != sizeof(struct R15) * 1000 + _Alignof(struct R15)) abi_fail(-1, 15);
    if (abi_layout(16) != sizeof(struct R16) * 1000 + _Alignof(struct R16)) abi_fail(-1, 16);
    if (abi_layout(17) != sizeof(union R17) * 1000 + _Alignof(union R17)) abi_fail(-1, 17);
    if (abi_layout(18) != sizeof(struct R18) * 1000 + _Alignof(struct R18)) abi_fail(-1, 18);
    if (abi_layout(19) != sizeof(struct R19) * 1000 + _Alignof(struct R19)) abi_fail(-1, 19);
    if (abi_layout(20) != sizeof(struct R20) * 1000 + _Alignof(struct R20)) abi_fail(-1, 20);
    if (abi_layout(21) != sizeof(struct R21) * 1000 + _Alignof(struct R21)) abi_fail(-1, 21);
    if (abi_layout(22) != sizeof(struct R22) * 1000 + _Alignof(struct R22)) abi_fail(-1, 22);
    if (abi_layout(23) != sizeof(struct R23) * 1000 + _Alignof(struct R23)) abi_fail(-1, 23);
    if (abi_layout(24) != sizeof(struct R24) * 1000 + _Alignof(struct R24)) abi_fail(-1, 24);
    if (abi_layout(25) != sizeof(struct R25) * 1000 + _Alignof(struct R25)) abi_fail(-1, 25);
    if (abi_layout(26) != sizeof(struct R26) * 1000 + _Alignof(struct R26)) abi_fail(-1, 26);
    if (abi_layout(27) != sizeof(struct R27) * 1000 + _Alignof(struct R27)) abi_fail(-1, 27);
    if (abi_layout(28) != sizeof(struct R28) * 1000 + _Alignof(struct R28)) abi_fail(-1, 28);
    if (abi_layout(29) != sizeof(struct R29) * 1000 + _Alignof(struct R29)) abi_fail(-1, 29);
    if (abi_layout(30) != sizeof(struct R30) * 1000 + _Alignof(struct R30)) abi_fail(-1, 30);
    if (abi_layout(31) != sizeof(struct R31) * 1000 + _Alignof(struct R31)) abi_fail(-1, 31);
    if (abi_layout(32) != sizeof(struct R32) * 1000 + _Alignof(struct R32)) abi_fail(-1, 32);
    if (abi_layout(33) != sizeof(union R33) * 1000 + _Alignof(union R33)) abi_fail(-1, 33);
    if (abi_layout(34) != sizeof(struct R34) * 1000 + _Alignof(struct R34)) abi_fail(-1, 34);
    if (abi_layout(35) != sizeof(struct R35) * 1000 + _Alignof(struct R35)) abi_fail(-1, 35);
    if (abi_layout(36) != sizeof(struct R36) * 1000 + _Alignof(struct R36)) abi_fail(-1, 36);
    if (abi_layout(37) != sizeof(struct R37) * 1000 + _Alignof(struct R37)) abi_fail(-1, 37);
    if (abi_layout(38) != sizeof(union R38) * 1000 + _Alignof(union R38)) abi_fail(-1, 38);
    if (abi_layout(39) != sizeof(union R39) * 1000 + _Alignof(union R39)) abi_fail(-1, 39);
    if (abi_layout(40) != sizeof(struct R40) * 1000 + _Alignof(struct R40)) abi_fail(-1, 40);
    if (abi_layout(41) != sizeof(struct R41) * 1000 + _Alignof(struct R41)) abi_fail(-1, 41);
    if (abi_layout(42) != sizeof(struct R42) * 1000 + _Alignof(struct R42)) abi_fail(-1, 42);
    if (abi_layout(43) != sizeof(struct R43) * 1000 + _Alignof(struct R43)) abi_fail(-1, 43);
    if (abi_layout(44) != sizeof(struct R44) * 1000 + _Alignof(struct R44)) abi_fail(-1, 44);
    if (abi_layout(45) != sizeof(struct R45) * 1000 + _Alignof(struct R45)) abi_fail(-1, 45);
    if (abi_layout(46) != sizeof(struct R46) * 1000 + _Alignof(struct R46)) abi_fail(-1, 46);
    if (abi_layout(47) != sizeof(struct R47) * 1000 + _Alignof(struct R47)) abi_fail(-1, 47);
    calls += abi_t0();
    calls += abi_t1();
    calls += abi_t2();
    calls += abi_t3();
    calls += abi_t4();
    calls += abi_t5();
    calls += abi_t6();
    calls += abi_t7();
    if (abi_result() != 0) print_i64(abi_first());
    print_i64(abi_result());
    print_i64(calls);
    return 0;
}
/* expect: 0 */
/* expect: 16 */
