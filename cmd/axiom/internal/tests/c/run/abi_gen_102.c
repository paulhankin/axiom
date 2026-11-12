/* abi_gen_102.c - written by gen_c_abi.py 102 8: calls of the
   functions of abi_gen_102_callee.c, directly, through pointers and with
   variadic arguments; prints the failures and the calls made. */
/* units: abi/abi_gen_102_callee.c */
#include "host.h"
#include "abi/abi_gen_102.h"

int abi_result(void);
int abi_first(void);
unsigned long long abi_layout(int k);

static void (*volatile abi_p0)(struct R0 a0, struct R1 a1, ...) = abi_f0;
static void (*volatile abi_p1)(double a0, int a1, int a2, double a3, int a4, int a5, struct R6 a6, unsigned long long a7, unsigned long long a8, struct R7 a9, struct R8 a10, struct R9 a11, struct R11 a12) = abi_f1;
static float (*volatile abi_p2)(void) = abi_f2;
static struct R15 (*volatile abi_p3)(float a0, float a1, struct R22 a2, _Bool a3, struct R23 a4, long long a5, double a6) = abi_f3;
static struct R24 (*volatile abi_p4)(struct R25 a0) = abi_f4;
static struct R26 (*volatile abi_p5)(union R28 a0, struct R30 a1, ...) = abi_f5;
static struct R39 (*volatile abi_p6)(char * a0, struct R41 a1, _Bool a2, char * a3, struct R42 a4, struct R44 a5, struct R46 a6, struct R47 a7, ...) = abi_f6;
static long long (*volatile abi_p7)(double a0, unsigned a1, unsigned long long a2, struct R56 a3, int a4, double a5, union R57 a6, struct R58 a7, struct R59 a8, ...) = abi_f7;

static int abi_t0(void) {
    struct R0 a0; fill_R0(&a0, 0u);
    struct R1 a1; fill_R1(&a1, 1u);
    struct R4 v0; fill_R4(&v0, 50u);
    unsigned long long v1 = (unsigned long long)abi_mix(51u, 0u);
    int bad = 0;
    abi_f0(a0, a1, v0, v1);
    abi_p0(a0, a1, v0, v1);
    if (bad) abi_fail(0, 99);
    return 2;
}

static int abi_t1(void) {
    double a0 = (double)((double)(long long)(abi_mix(100u, 0u) % 4001u) / 8.0 - 250.0);
    int a1 = (int)((long long)(abi_mix(101u, 0u) % 4294967296u) - 2147483648);
    int a2 = (int)((long long)(abi_mix(102u, 0u) % 4294967296u) - 2147483648);
    double a3 = (double)((double)(long long)(abi_mix(103u, 0u) % 4001u) / 8.0 - 250.0);
    int a4 = (int)((long long)(abi_mix(104u, 0u) % 4294967296u) - 2147483648);
    int a5 = (int)((long long)(abi_mix(105u, 0u) % 4294967296u) - 2147483648);
    struct R6 a6; fill_R6(&a6, 106u);
    unsigned long long a7 = (unsigned long long)abi_mix(107u, 0u);
    unsigned long long a8 = (unsigned long long)abi_mix(108u, 0u);
    struct R7 a9; fill_R7(&a9, 109u);
    struct R8 a10; fill_R8(&a10, 110u);
    struct R9 a11; fill_R9(&a11, 111u);
    struct R11 a12; fill_R11(&a12, 112u);
    int bad = 0;
    abi_f1(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    abi_p1(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
    if (bad) abi_fail(1, 99);
    return 2;
}

static int abi_t2(void) {
    int bad = 0;
    {
        float r = abi_f2();
        if (!(r == (float)((double)(long long)(abi_mix(299u, 0u) % 4001u) / 8.0 - 250.0))) bad = 1;
    }
    {
        float r = abi_p2();
        if (!(r == (float)((double)(long long)(abi_mix(299u, 0u) % 4001u) / 8.0 - 250.0))) bad = 1;
    }
    if (bad) abi_fail(2, 99);
    return 2;
}

static int abi_t3(void) {
    float a0 = (float)((double)(long long)(abi_mix(300u, 0u) % 4001u) / 8.0 - 250.0);
    float a1 = (float)((double)(long long)(abi_mix(301u, 0u) % 4001u) / 8.0 - 250.0);
    struct R22 a2; fill_R22(&a2, 302u);
    _Bool a3 = (_Bool)(abi_mix(303u, 0u) & 1u);
    struct R23 a4; fill_R23(&a4, 304u);
    long long a5 = (long long)abi_mix(305u, 0u);
    double a6 = (double)((double)(long long)(abi_mix(306u, 0u) % 4001u) / 8.0 - 250.0);
    int bad = 0;
    {
        struct R15 r = abi_f3(a0, a1, a2, a3, a4, a5, a6);
        if (!(check_R15(&r, 399u))) bad = 1;
    }
    {
        struct R15 r = abi_p3(a0, a1, a2, a3, a4, a5, a6);
        if (!(check_R15(&r, 399u))) bad = 1;
    }
    if (bad) abi_fail(3, 99);
    return 2;
}

static int abi_t4(void) {
    struct R25 a0; fill_R25(&a0, 400u);
    int bad = 0;
    {
        struct R24 r = abi_f4(a0);
        if (!(check_R24(&r, 499u))) bad = 1;
    }
    {
        struct R24 r = abi_p4(a0);
        if (!(check_R24(&r, 499u))) bad = 1;
    }
    if (bad) abi_fail(4, 99);
    return 2;
}

static int abi_t5(void) {
    union R28 a0; fill_R28(&a0, 500u);
    struct R30 a1; fill_R30(&a1, 501u);
    struct R31 v0; fill_R31(&v0, 550u);
    struct R33 v1; fill_R33(&v1, 551u);
    struct R34 v2; fill_R34(&v2, 552u);
    int bad = 0;
    {
        struct R26 r = abi_f5(a0, a1, v0, v1, v2);
        if (!(check_R26(&r, 599u))) bad = 1;
    }
    {
        struct R26 r = abi_p5(a0, a1, v0, v1, v2);
        if (!(check_R26(&r, 599u))) bad = 1;
    }
    if (bad) abi_fail(5, 99);
    return 2;
}

static int abi_t6(void) {
    char * a0 = (abi_anchor + (abi_mix(600u, 0u) % 8u));
    struct R41 a1; fill_R41(&a1, 601u);
    _Bool a2 = (_Bool)(abi_mix(602u, 0u) & 1u);
    char * a3 = (abi_anchor + (abi_mix(603u, 0u) % 8u));
    struct R42 a4; fill_R42(&a4, 604u);
    struct R44 a5; fill_R44(&a5, 605u);
    struct R46 a6; fill_R46(&a6, 606u);
    struct R47 a7; fill_R47(&a7, 607u);
    union R50 v0; fill_R50(&v0, 650u);
    struct R51 v1; fill_R51(&v1, 651u);
    struct R52 v2; fill_R52(&v2, 652u);
    struct R53 v3; fill_R53(&v3, 653u);
    struct R54 v4; fill_R54(&v4, 654u);
    struct R55 v5; fill_R55(&v5, 655u);
    int bad = 0;
    {
        struct R39 r = abi_f6(a0, a1, a2, a3, a4, a5, a6, a7, v0, v1, v2, v3, v4, v5);
        if (!(check_R39(&r, 699u))) bad = 1;
    }
    {
        struct R39 r = abi_p6(a0, a1, a2, a3, a4, a5, a6, a7, v0, v1, v2, v3, v4, v5);
        if (!(check_R39(&r, 699u))) bad = 1;
    }
    if (bad) abi_fail(6, 99);
    return 2;
}

static int abi_t7(void) {
    double a0 = (double)((double)(long long)(abi_mix(700u, 0u) % 4001u) / 8.0 - 250.0);
    unsigned a1 = (unsigned)(abi_mix(701u, 0u) % 4294967296u);
    unsigned long long a2 = (unsigned long long)abi_mix(702u, 0u);
    struct R56 a3; fill_R56(&a3, 703u);
    int a4 = (int)((long long)(abi_mix(704u, 0u) % 4294967296u) - 2147483648);
    double a5 = (double)((double)(long long)(abi_mix(705u, 0u) % 4001u) / 8.0 - 250.0);
    union R57 a6; fill_R57(&a6, 706u);
    struct R58 a7; fill_R58(&a7, 707u);
    struct R59 a8; fill_R59(&a8, 708u);
    union R62 v0; fill_R62(&v0, 750u);
    struct R63 v1; fill_R63(&v1, 751u);
    struct R64 v2; fill_R64(&v2, 752u);
    struct R65 v3; fill_R65(&v3, 753u);
    int bad = 0;
    {
        long long r = abi_f7(a0, a1, a2, a3, a4, a5, a6, a7, a8, v0, v1, v2, v3);
        if (!(r == (long long)abi_mix(799u, 0u))) bad = 1;
    }
    {
        long long r = abi_p7(a0, a1, a2, a3, a4, a5, a6, a7, a8, v0, v1, v2, v3);
        if (!(r == (long long)abi_mix(799u, 0u))) bad = 1;
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
    if (abi_layout(12) != sizeof(struct R12) * 1000 + _Alignof(struct R12)) abi_fail(-1, 12);
    if (abi_layout(13) != sizeof(struct R13) * 1000 + _Alignof(struct R13)) abi_fail(-1, 13);
    if (abi_layout(14) != sizeof(union R14) * 1000 + _Alignof(union R14)) abi_fail(-1, 14);
    if (abi_layout(15) != sizeof(struct R15) * 1000 + _Alignof(struct R15)) abi_fail(-1, 15);
    if (abi_layout(16) != sizeof(struct R16) * 1000 + _Alignof(struct R16)) abi_fail(-1, 16);
    if (abi_layout(17) != sizeof(union R17) * 1000 + _Alignof(union R17)) abi_fail(-1, 17);
    if (abi_layout(18) != sizeof(struct R18) * 1000 + _Alignof(struct R18)) abi_fail(-1, 18);
    if (abi_layout(19) != sizeof(struct R19) * 1000 + _Alignof(struct R19)) abi_fail(-1, 19);
    if (abi_layout(20) != sizeof(struct R20) * 1000 + _Alignof(struct R20)) abi_fail(-1, 20);
    if (abi_layout(21) != sizeof(union R21) * 1000 + _Alignof(union R21)) abi_fail(-1, 21);
    if (abi_layout(22) != sizeof(struct R22) * 1000 + _Alignof(struct R22)) abi_fail(-1, 22);
    if (abi_layout(23) != sizeof(struct R23) * 1000 + _Alignof(struct R23)) abi_fail(-1, 23);
    if (abi_layout(24) != sizeof(struct R24) * 1000 + _Alignof(struct R24)) abi_fail(-1, 24);
    if (abi_layout(25) != sizeof(struct R25) * 1000 + _Alignof(struct R25)) abi_fail(-1, 25);
    if (abi_layout(26) != sizeof(struct R26) * 1000 + _Alignof(struct R26)) abi_fail(-1, 26);
    if (abi_layout(27) != sizeof(struct R27) * 1000 + _Alignof(struct R27)) abi_fail(-1, 27);
    if (abi_layout(28) != sizeof(union R28) * 1000 + _Alignof(union R28)) abi_fail(-1, 28);
    if (abi_layout(29) != sizeof(struct R29) * 1000 + _Alignof(struct R29)) abi_fail(-1, 29);
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
    if (abi_layout(41) != sizeof(struct R41) * 1000 + _Alignof(struct R41)) abi_fail(-1, 41);
    if (abi_layout(42) != sizeof(struct R42) * 1000 + _Alignof(struct R42)) abi_fail(-1, 42);
    if (abi_layout(43) != sizeof(struct R43) * 1000 + _Alignof(struct R43)) abi_fail(-1, 43);
    if (abi_layout(44) != sizeof(struct R44) * 1000 + _Alignof(struct R44)) abi_fail(-1, 44);
    if (abi_layout(45) != sizeof(struct R45) * 1000 + _Alignof(struct R45)) abi_fail(-1, 45);
    if (abi_layout(46) != sizeof(struct R46) * 1000 + _Alignof(struct R46)) abi_fail(-1, 46);
    if (abi_layout(47) != sizeof(struct R47) * 1000 + _Alignof(struct R47)) abi_fail(-1, 47);
    if (abi_layout(48) != sizeof(struct R48) * 1000 + _Alignof(struct R48)) abi_fail(-1, 48);
    if (abi_layout(49) != sizeof(struct R49) * 1000 + _Alignof(struct R49)) abi_fail(-1, 49);
    if (abi_layout(50) != sizeof(union R50) * 1000 + _Alignof(union R50)) abi_fail(-1, 50);
    if (abi_layout(51) != sizeof(struct R51) * 1000 + _Alignof(struct R51)) abi_fail(-1, 51);
    if (abi_layout(52) != sizeof(struct R52) * 1000 + _Alignof(struct R52)) abi_fail(-1, 52);
    if (abi_layout(53) != sizeof(struct R53) * 1000 + _Alignof(struct R53)) abi_fail(-1, 53);
    if (abi_layout(54) != sizeof(struct R54) * 1000 + _Alignof(struct R54)) abi_fail(-1, 54);
    if (abi_layout(55) != sizeof(struct R55) * 1000 + _Alignof(struct R55)) abi_fail(-1, 55);
    if (abi_layout(56) != sizeof(struct R56) * 1000 + _Alignof(struct R56)) abi_fail(-1, 56);
    if (abi_layout(57) != sizeof(union R57) * 1000 + _Alignof(union R57)) abi_fail(-1, 57);
    if (abi_layout(58) != sizeof(struct R58) * 1000 + _Alignof(struct R58)) abi_fail(-1, 58);
    if (abi_layout(59) != sizeof(struct R59) * 1000 + _Alignof(struct R59)) abi_fail(-1, 59);
    if (abi_layout(60) != sizeof(struct R60) * 1000 + _Alignof(struct R60)) abi_fail(-1, 60);
    if (abi_layout(61) != sizeof(struct R61) * 1000 + _Alignof(struct R61)) abi_fail(-1, 61);
    if (abi_layout(62) != sizeof(union R62) * 1000 + _Alignof(union R62)) abi_fail(-1, 62);
    if (abi_layout(63) != sizeof(struct R63) * 1000 + _Alignof(struct R63)) abi_fail(-1, 63);
    if (abi_layout(64) != sizeof(struct R64) * 1000 + _Alignof(struct R64)) abi_fail(-1, 64);
    if (abi_layout(65) != sizeof(struct R65) * 1000 + _Alignof(struct R65)) abi_fail(-1, 65);
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
