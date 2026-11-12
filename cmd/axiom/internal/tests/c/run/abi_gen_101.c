/* abi_gen_101.c - written by gen_c_abi.py 101 8: calls of the
   functions of abi_gen_101_callee.c, directly, through pointers and with
   variadic arguments; prints the failures and the calls made. */
/* units: abi/abi_gen_101_callee.c */
#include "host.h"
#include "abi/abi_gen_101.h"

int abi_result(void);
int abi_first(void);
unsigned long long abi_layout(int k);

static float (*volatile abi_p0)(float a0, unsigned short a1, struct R0 a2, struct R1 a3, unsigned a4, signed char a5, struct R2 a6, double a7, struct R3 a8, struct R4 a9) = abi_f0;
static _Bool (*volatile abi_p1)(struct R5 a0, struct R6 a1, int a2, int a3, struct R9 a4) = abi_f1;
static struct R10 (*volatile abi_p2)(union R13 a0, struct R14 a1, struct R15 a2, struct R17 a3, float a4) = abi_f2;
static struct R18 (*volatile abi_p3)(struct R19 a0, struct R20 a1, struct R22 a2, struct R24 a3, struct R27 a4) = abi_f3;
static struct R31 (*volatile abi_p4)(struct R32 a0, struct R34 a1, struct R39 a2, struct R40 a3, ...) = abi_f4;
static void (*volatile abi_p5)(union R41 a0, unsigned char a1, struct R43 a2) = abi_f5;
static struct R44 (*volatile abi_p6)(union R47 a0, struct R48 a1, _Bool a2, struct R49 a3, struct R50 a4, float a5, char * a6, struct R51 a7, struct R52 a8, double a9) = abi_f6;
static union R53 (*volatile abi_p7)(double a0, struct R54 a1, struct R55 a2, unsigned a3, signed char a4, signed char a5, struct R57 a6, struct R58 a7, struct R59 a8, unsigned char a9) = abi_f7;

static int abi_t0(void) {
    float a0 = (float)((double)(long long)(abi_mix(0u, 0u) % 4001u) / 8.0 - 250.0);
    unsigned short a1 = (unsigned short)(abi_mix(1u, 0u) % 65536u);
    struct R0 a2; fill_R0(&a2, 2u);
    struct R1 a3; fill_R1(&a3, 3u);
    unsigned a4 = (unsigned)(abi_mix(4u, 0u) % 4294967296u);
    signed char a5 = (signed char)((long long)(abi_mix(5u, 0u) % 256u) - 128);
    struct R2 a6; fill_R2(&a6, 6u);
    double a7 = (double)((double)(long long)(abi_mix(7u, 0u) % 4001u) / 8.0 - 250.0);
    struct R3 a8; fill_R3(&a8, 8u);
    struct R4 a9; fill_R4(&a9, 9u);
    int bad = 0;
    {
        float r = abi_f0(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(r == (float)((double)(long long)(abi_mix(99u, 0u) % 4001u) / 8.0 - 250.0))) bad = 1;
    }
    {
        float r = abi_p0(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(r == (float)((double)(long long)(abi_mix(99u, 0u) % 4001u) / 8.0 - 250.0))) bad = 1;
    }
    if (bad) abi_fail(0, 99);
    return 2;
}

static int abi_t1(void) {
    struct R5 a0; fill_R5(&a0, 100u);
    struct R6 a1; fill_R6(&a1, 101u);
    int a2 = (int)((long long)(abi_mix(102u, 0u) % 4294967296u) - 2147483648);
    int a3 = (int)((long long)(abi_mix(103u, 0u) % 4294967296u) - 2147483648);
    struct R9 a4; fill_R9(&a4, 104u);
    int bad = 0;
    {
        _Bool r = abi_f1(a0, a1, a2, a3, a4);
        if (!(r == (_Bool)(abi_mix(199u, 0u) & 1u))) bad = 1;
    }
    {
        _Bool r = abi_p1(a0, a1, a2, a3, a4);
        if (!(r == (_Bool)(abi_mix(199u, 0u) & 1u))) bad = 1;
    }
    if (bad) abi_fail(1, 99);
    return 2;
}

static int abi_t2(void) {
    union R13 a0; fill_R13(&a0, 200u);
    struct R14 a1; fill_R14(&a1, 201u);
    struct R15 a2; fill_R15(&a2, 202u);
    struct R17 a3; fill_R17(&a3, 203u);
    float a4 = (float)((double)(long long)(abi_mix(204u, 0u) % 4001u) / 8.0 - 250.0);
    int bad = 0;
    {
        struct R10 r = abi_f2(a0, a1, a2, a3, a4);
        if (!(check_R10(&r, 299u))) bad = 1;
    }
    {
        struct R10 r = abi_p2(a0, a1, a2, a3, a4);
        if (!(check_R10(&r, 299u))) bad = 1;
    }
    if (bad) abi_fail(2, 99);
    return 2;
}

static int abi_t3(void) {
    struct R19 a0; fill_R19(&a0, 300u);
    struct R20 a1; fill_R20(&a1, 301u);
    struct R22 a2; fill_R22(&a2, 302u);
    struct R24 a3; fill_R24(&a3, 303u);
    struct R27 a4; fill_R27(&a4, 304u);
    int bad = 0;
    {
        struct R18 r = abi_f3(a0, a1, a2, a3, a4);
        if (!(check_R18(&r, 399u))) bad = 1;
    }
    {
        struct R18 r = abi_p3(a0, a1, a2, a3, a4);
        if (!(check_R18(&r, 399u))) bad = 1;
    }
    if (bad) abi_fail(3, 99);
    return 2;
}

static int abi_t4(void) {
    struct R32 a0; fill_R32(&a0, 400u);
    struct R34 a1; fill_R34(&a1, 401u);
    struct R39 a2; fill_R39(&a2, 402u);
    struct R40 a3; fill_R40(&a3, 403u);
    double v0 = (double)((double)(long long)(abi_mix(450u, 0u) % 4001u) / 8.0 - 250.0);
    unsigned long long v1 = (unsigned long long)abi_mix(451u, 0u);
    unsigned long long v2 = (unsigned long long)abi_mix(452u, 0u);
    int v3 = (int)((long long)(abi_mix(453u, 0u) % 4294967296u) - 2147483648);
    int bad = 0;
    {
        struct R31 r = abi_f4(a0, a1, a2, a3, v0, v1, v2, v3);
        if (!(check_R31(&r, 499u))) bad = 1;
    }
    {
        struct R31 r = abi_p4(a0, a1, a2, a3, v0, v1, v2, v3);
        if (!(check_R31(&r, 499u))) bad = 1;
    }
    if (bad) abi_fail(4, 99);
    return 2;
}

static int abi_t5(void) {
    union R41 a0; fill_R41(&a0, 500u);
    unsigned char a1 = (unsigned char)(abi_mix(501u, 0u) % 256u);
    struct R43 a2; fill_R43(&a2, 502u);
    int bad = 0;
    abi_f5(a0, a1, a2);
    abi_p5(a0, a1, a2);
    if (bad) abi_fail(5, 99);
    return 2;
}

static int abi_t6(void) {
    union R47 a0; fill_R47(&a0, 600u);
    struct R48 a1; fill_R48(&a1, 601u);
    _Bool a2 = (_Bool)(abi_mix(602u, 0u) & 1u);
    struct R49 a3; fill_R49(&a3, 603u);
    struct R50 a4; fill_R50(&a4, 604u);
    float a5 = (float)((double)(long long)(abi_mix(605u, 0u) % 4001u) / 8.0 - 250.0);
    char * a6 = (abi_anchor + (abi_mix(606u, 0u) % 8u));
    struct R51 a7; fill_R51(&a7, 607u);
    struct R52 a8; fill_R52(&a8, 608u);
    double a9 = (double)((double)(long long)(abi_mix(609u, 0u) % 4001u) / 8.0 - 250.0);
    int bad = 0;
    {
        struct R44 r = abi_f6(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(check_R44(&r, 699u))) bad = 1;
    }
    {
        struct R44 r = abi_p6(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(check_R44(&r, 699u))) bad = 1;
    }
    if (bad) abi_fail(6, 99);
    return 2;
}

static int abi_t7(void) {
    double a0 = (double)((double)(long long)(abi_mix(700u, 0u) % 4001u) / 8.0 - 250.0);
    struct R54 a1; fill_R54(&a1, 701u);
    struct R55 a2; fill_R55(&a2, 702u);
    unsigned a3 = (unsigned)(abi_mix(703u, 0u) % 4294967296u);
    signed char a4 = (signed char)((long long)(abi_mix(704u, 0u) % 256u) - 128);
    signed char a5 = (signed char)((long long)(abi_mix(705u, 0u) % 256u) - 128);
    struct R57 a6; fill_R57(&a6, 706u);
    struct R58 a7; fill_R58(&a7, 707u);
    struct R59 a8; fill_R59(&a8, 708u);
    unsigned char a9 = (unsigned char)(abi_mix(709u, 0u) % 256u);
    int bad = 0;
    {
        union R53 r = abi_f7(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(check_R53(&r, 799u))) bad = 1;
    }
    {
        union R53 r = abi_p7(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(check_R53(&r, 799u))) bad = 1;
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
    if (abi_layout(5) != sizeof(struct R5) * 1000 + _Alignof(struct R5)) abi_fail(-1, 5);
    if (abi_layout(6) != sizeof(struct R6) * 1000 + _Alignof(struct R6)) abi_fail(-1, 6);
    if (abi_layout(7) != sizeof(struct R7) * 1000 + _Alignof(struct R7)) abi_fail(-1, 7);
    if (abi_layout(8) != sizeof(struct R8) * 1000 + _Alignof(struct R8)) abi_fail(-1, 8);
    if (abi_layout(9) != sizeof(struct R9) * 1000 + _Alignof(struct R9)) abi_fail(-1, 9);
    if (abi_layout(10) != sizeof(struct R10) * 1000 + _Alignof(struct R10)) abi_fail(-1, 10);
    if (abi_layout(11) != sizeof(struct R11) * 1000 + _Alignof(struct R11)) abi_fail(-1, 11);
    if (abi_layout(12) != sizeof(union R12) * 1000 + _Alignof(union R12)) abi_fail(-1, 12);
    if (abi_layout(13) != sizeof(union R13) * 1000 + _Alignof(union R13)) abi_fail(-1, 13);
    if (abi_layout(14) != sizeof(struct R14) * 1000 + _Alignof(struct R14)) abi_fail(-1, 14);
    if (abi_layout(15) != sizeof(struct R15) * 1000 + _Alignof(struct R15)) abi_fail(-1, 15);
    if (abi_layout(16) != sizeof(struct R16) * 1000 + _Alignof(struct R16)) abi_fail(-1, 16);
    if (abi_layout(17) != sizeof(struct R17) * 1000 + _Alignof(struct R17)) abi_fail(-1, 17);
    if (abi_layout(18) != sizeof(struct R18) * 1000 + _Alignof(struct R18)) abi_fail(-1, 18);
    if (abi_layout(19) != sizeof(struct R19) * 1000 + _Alignof(struct R19)) abi_fail(-1, 19);
    if (abi_layout(20) != sizeof(struct R20) * 1000 + _Alignof(struct R20)) abi_fail(-1, 20);
    if (abi_layout(21) != sizeof(struct R21) * 1000 + _Alignof(struct R21)) abi_fail(-1, 21);
    if (abi_layout(22) != sizeof(struct R22) * 1000 + _Alignof(struct R22)) abi_fail(-1, 22);
    if (abi_layout(23) != sizeof(struct R23) * 1000 + _Alignof(struct R23)) abi_fail(-1, 23);
    if (abi_layout(24) != sizeof(struct R24) * 1000 + _Alignof(struct R24)) abi_fail(-1, 24);
    if (abi_layout(25) != sizeof(struct R25) * 1000 + _Alignof(struct R25)) abi_fail(-1, 25);
    if (abi_layout(26) != sizeof(union R26) * 1000 + _Alignof(union R26)) abi_fail(-1, 26);
    if (abi_layout(27) != sizeof(struct R27) * 1000 + _Alignof(struct R27)) abi_fail(-1, 27);
    if (abi_layout(28) != sizeof(struct R28) * 1000 + _Alignof(struct R28)) abi_fail(-1, 28);
    if (abi_layout(29) != sizeof(union R29) * 1000 + _Alignof(union R29)) abi_fail(-1, 29);
    if (abi_layout(30) != sizeof(struct R30) * 1000 + _Alignof(struct R30)) abi_fail(-1, 30);
    if (abi_layout(31) != sizeof(struct R31) * 1000 + _Alignof(struct R31)) abi_fail(-1, 31);
    if (abi_layout(32) != sizeof(struct R32) * 1000 + _Alignof(struct R32)) abi_fail(-1, 32);
    if (abi_layout(33) != sizeof(struct R33) * 1000 + _Alignof(struct R33)) abi_fail(-1, 33);
    if (abi_layout(34) != sizeof(struct R34) * 1000 + _Alignof(struct R34)) abi_fail(-1, 34);
    if (abi_layout(35) != sizeof(struct R35) * 1000 + _Alignof(struct R35)) abi_fail(-1, 35);
    if (abi_layout(36) != sizeof(struct R36) * 1000 + _Alignof(struct R36)) abi_fail(-1, 36);
    if (abi_layout(37) != sizeof(struct R37) * 1000 + _Alignof(struct R37)) abi_fail(-1, 37);
    if (abi_layout(38) != sizeof(struct R38) * 1000 + _Alignof(struct R38)) abi_fail(-1, 38);
    if (abi_layout(39) != sizeof(struct R39) * 1000 + _Alignof(struct R39)) abi_fail(-1, 39);
    if (abi_layout(40) != sizeof(struct R40) * 1000 + _Alignof(struct R40)) abi_fail(-1, 40);
    if (abi_layout(41) != sizeof(union R41) * 1000 + _Alignof(union R41)) abi_fail(-1, 41);
    if (abi_layout(42) != sizeof(struct R42) * 1000 + _Alignof(struct R42)) abi_fail(-1, 42);
    if (abi_layout(43) != sizeof(struct R43) * 1000 + _Alignof(struct R43)) abi_fail(-1, 43);
    if (abi_layout(44) != sizeof(struct R44) * 1000 + _Alignof(struct R44)) abi_fail(-1, 44);
    if (abi_layout(45) != sizeof(struct R45) * 1000 + _Alignof(struct R45)) abi_fail(-1, 45);
    if (abi_layout(46) != sizeof(struct R46) * 1000 + _Alignof(struct R46)) abi_fail(-1, 46);
    if (abi_layout(47) != sizeof(union R47) * 1000 + _Alignof(union R47)) abi_fail(-1, 47);
    if (abi_layout(48) != sizeof(struct R48) * 1000 + _Alignof(struct R48)) abi_fail(-1, 48);
    if (abi_layout(49) != sizeof(struct R49) * 1000 + _Alignof(struct R49)) abi_fail(-1, 49);
    if (abi_layout(50) != sizeof(struct R50) * 1000 + _Alignof(struct R50)) abi_fail(-1, 50);
    if (abi_layout(51) != sizeof(struct R51) * 1000 + _Alignof(struct R51)) abi_fail(-1, 51);
    if (abi_layout(52) != sizeof(struct R52) * 1000 + _Alignof(struct R52)) abi_fail(-1, 52);
    if (abi_layout(53) != sizeof(union R53) * 1000 + _Alignof(union R53)) abi_fail(-1, 53);
    if (abi_layout(54) != sizeof(struct R54) * 1000 + _Alignof(struct R54)) abi_fail(-1, 54);
    if (abi_layout(55) != sizeof(struct R55) * 1000 + _Alignof(struct R55)) abi_fail(-1, 55);
    if (abi_layout(56) != sizeof(struct R56) * 1000 + _Alignof(struct R56)) abi_fail(-1, 56);
    if (abi_layout(57) != sizeof(struct R57) * 1000 + _Alignof(struct R57)) abi_fail(-1, 57);
    if (abi_layout(58) != sizeof(struct R58) * 1000 + _Alignof(struct R58)) abi_fail(-1, 58);
    if (abi_layout(59) != sizeof(struct R59) * 1000 + _Alignof(struct R59)) abi_fail(-1, 59);
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
