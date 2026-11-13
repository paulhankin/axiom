/* abi_gen_105.c - written by gen_c_abi.py 105 8: calls of the
   functions of abi_gen_105_callee.c, directly, through pointers and with
   variadic arguments; prints the failures and the calls made. */
/* flags: --c-std=c23 */
/* units: abi/abi_gen_105_callee.c */
#include "host.h"
#include "abi/abi_gen_105.h"

int abi_result(void);
int abi_first(void);
unsigned long long abi_layout(int k);

static struct R0 (*volatile abi_p0)(struct R2 a0, ...) = abi_f0;
static signed char (*volatile abi_p1)(int a0, float a1, struct R7 a2, _Decimal128 a3, union R9 a4, struct R10 a5, struct R12 a6, struct R13 a7, struct R17 a8, unsigned long long a9, struct R19 a10, _Decimal32 a11, ...) = abi_f1;
static struct R23 (*volatile abi_p2)(int a0, long long a1, int a2, long long a3, int a4, float a5, float a6, struct R25 a7, struct R26 a8, struct R27 a9, struct R28 a10, struct R30 a11, struct R34 a12, struct R35 a13, struct R36 a14, struct R37 a15, _Decimal32 a16) = abi_f2;
static union R38 (*volatile abi_p3)(struct R39 a0, struct R40 a1, struct R41 a2, _Bool a3) = abi_f3;
static struct R42 (*volatile abi_p4)(int a0, float a1, long long a2, int a3, int a4, float a5, int a6, int a7, struct R43 a8, struct R46 a9) = abi_f4;
static union R47 (*volatile abi_p5)(_Decimal32 a0, char * a1, unsigned short a2, struct R49 a3, struct R50 a4, struct R54 a5, double a6, struct R55 a7, struct R56 a8, struct R61 a9) = abi_f5;
static struct R62 (*volatile abi_p6)(struct R64 a0, struct R66 a1, struct R67 a2, unsigned long long a3, ...) = abi_f6;
static void (*volatile abi_p7)(struct R68 a0, struct R69 a1, ...) = abi_f7;

static int abi_t0(void) {
    struct R2 a0; fill_R2(&a0, 0u);
    struct R6 v0; fill_R6(&v0, 50u);
    int bad = 0;
    {
        struct R0 r = abi_f0(a0, v0);
        if (!(check_R0(&r, 99u))) bad = 1;
    }
    {
        struct R0 r = abi_p0(a0, v0);
        if (!(check_R0(&r, 99u))) bad = 1;
    }
    if (bad) abi_fail(0, 99);
    return 2;
}

static int abi_t1(void) {
    int a0 = (int)((long long)(abi_mix(100u, 0u) % 4294967296u) - 2147483648);
    float a1 = (float)((double)(long long)(abi_mix(101u, 0u) % 4001u) / 8.0 - 250.0);
    struct R7 a2; fill_R7(&a2, 102u);
    _Decimal128 a3 = (_Decimal128)((double)(long long)(abi_mix(103u, 0u) % 4001u) / 8.0 - 250.0);
    union R9 a4; fill_R9(&a4, 104u);
    struct R10 a5; fill_R10(&a5, 105u);
    struct R12 a6; fill_R12(&a6, 106u);
    struct R13 a7; fill_R13(&a7, 107u);
    struct R17 a8; fill_R17(&a8, 108u);
    unsigned long long a9 = (unsigned long long)abi_mix(109u, 0u);
    struct R19 a10; fill_R19(&a10, 110u);
    _Decimal32 a11 = (_Decimal32)((double)(long long)(abi_mix(111u, 0u) % 4001u) / 8.0 - 250.0);
    struct R20 v0; fill_R20(&v0, 150u);
    struct R22 v1; fill_R22(&v1, 151u);
    int bad = 0;
    {
        signed char r = abi_f1(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, v0, v1);
        if (!(r == (signed char)((long long)(abi_mix(199u, 0u) % 256u) - 128))) bad = 1;
    }
    {
        signed char r = abi_p1(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, v0, v1);
        if (!(r == (signed char)((long long)(abi_mix(199u, 0u) % 256u) - 128))) bad = 1;
    }
    if (bad) abi_fail(1, 99);
    return 2;
}

static int abi_t2(void) {
    int a0 = (int)((long long)(abi_mix(200u, 0u) % 4294967296u) - 2147483648);
    long long a1 = (long long)abi_mix(201u, 0u);
    int a2 = (int)((long long)(abi_mix(202u, 0u) % 4294967296u) - 2147483648);
    long long a3 = (long long)abi_mix(203u, 0u);
    int a4 = (int)((long long)(abi_mix(204u, 0u) % 4294967296u) - 2147483648);
    float a5 = (float)((double)(long long)(abi_mix(205u, 0u) % 4001u) / 8.0 - 250.0);
    float a6 = (float)((double)(long long)(abi_mix(206u, 0u) % 4001u) / 8.0 - 250.0);
    struct R25 a7; fill_R25(&a7, 207u);
    struct R26 a8; fill_R26(&a8, 208u);
    struct R27 a9; fill_R27(&a9, 209u);
    struct R28 a10; fill_R28(&a10, 210u);
    struct R30 a11; fill_R30(&a11, 211u);
    struct R34 a12; fill_R34(&a12, 212u);
    struct R35 a13; fill_R35(&a13, 213u);
    struct R36 a14; fill_R36(&a14, 214u);
    struct R37 a15; fill_R37(&a15, 215u);
    _Decimal32 a16 = (_Decimal32)((double)(long long)(abi_mix(216u, 0u) % 4001u) / 8.0 - 250.0);
    int bad = 0;
    {
        struct R23 r = abi_f2(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
        if (!(check_R23(&r, 299u))) bad = 1;
    }
    {
        struct R23 r = abi_p2(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16);
        if (!(check_R23(&r, 299u))) bad = 1;
    }
    if (bad) abi_fail(2, 99);
    return 2;
}

static int abi_t3(void) {
    struct R39 a0; fill_R39(&a0, 300u);
    struct R40 a1; fill_R40(&a1, 301u);
    struct R41 a2; fill_R41(&a2, 302u);
    _Bool a3 = (_Bool)(abi_mix(303u, 0u) & 1u);
    int bad = 0;
    {
        union R38 r = abi_f3(a0, a1, a2, a3);
        if (!(check_R38(&r, 399u))) bad = 1;
    }
    {
        union R38 r = abi_p3(a0, a1, a2, a3);
        if (!(check_R38(&r, 399u))) bad = 1;
    }
    if (bad) abi_fail(3, 99);
    return 2;
}

static int abi_t4(void) {
    int a0 = (int)((long long)(abi_mix(400u, 0u) % 4294967296u) - 2147483648);
    float a1 = (float)((double)(long long)(abi_mix(401u, 0u) % 4001u) / 8.0 - 250.0);
    long long a2 = (long long)abi_mix(402u, 0u);
    int a3 = (int)((long long)(abi_mix(403u, 0u) % 4294967296u) - 2147483648);
    int a4 = (int)((long long)(abi_mix(404u, 0u) % 4294967296u) - 2147483648);
    float a5 = (float)((double)(long long)(abi_mix(405u, 0u) % 4001u) / 8.0 - 250.0);
    int a6 = (int)((long long)(abi_mix(406u, 0u) % 4294967296u) - 2147483648);
    int a7 = (int)((long long)(abi_mix(407u, 0u) % 4294967296u) - 2147483648);
    struct R43 a8; fill_R43(&a8, 408u);
    struct R46 a9; fill_R46(&a9, 409u);
    int bad = 0;
    {
        struct R42 r = abi_f4(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(check_R42(&r, 499u))) bad = 1;
    }
    {
        struct R42 r = abi_p4(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(check_R42(&r, 499u))) bad = 1;
    }
    if (bad) abi_fail(4, 99);
    return 2;
}

static int abi_t5(void) {
    _Decimal32 a0 = (_Decimal32)((double)(long long)(abi_mix(500u, 0u) % 4001u) / 8.0 - 250.0);
    char * a1 = (abi_anchor + (abi_mix(501u, 0u) % 8u));
    unsigned short a2 = (unsigned short)(abi_mix(502u, 0u) % 65536u);
    struct R49 a3; fill_R49(&a3, 503u);
    struct R50 a4; fill_R50(&a4, 504u);
    struct R54 a5; fill_R54(&a5, 505u);
    double a6 = (double)((double)(long long)(abi_mix(506u, 0u) % 4001u) / 8.0 - 250.0);
    struct R55 a7; fill_R55(&a7, 507u);
    struct R56 a8; fill_R56(&a8, 508u);
    struct R61 a9; fill_R61(&a9, 509u);
    int bad = 0;
    {
        union R47 r = abi_f5(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(check_R47(&r, 599u))) bad = 1;
    }
    {
        union R47 r = abi_p5(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
        if (!(check_R47(&r, 599u))) bad = 1;
    }
    if (bad) abi_fail(5, 99);
    return 2;
}

static int abi_t6(void) {
    struct R64 a0; fill_R64(&a0, 600u);
    struct R66 a1; fill_R66(&a1, 601u);
    struct R67 a2; fill_R67(&a2, 602u);
    unsigned long long a3 = (unsigned long long)abi_mix(603u, 0u);
    _Decimal128 v0 = (_Decimal128)((double)(long long)(abi_mix(650u, 0u) % 4001u) / 8.0 - 250.0);
    _Decimal32 v1 = (_Decimal32)((double)(long long)(abi_mix(651u, 0u) % 4001u) / 8.0 - 250.0);
    int bad = 0;
    {
        struct R62 r = abi_f6(a0, a1, a2, a3, v0, v1);
        if (!(check_R62(&r, 699u))) bad = 1;
    }
    {
        struct R62 r = abi_p6(a0, a1, a2, a3, v0, v1);
        if (!(check_R62(&r, 699u))) bad = 1;
    }
    if (bad) abi_fail(6, 99);
    return 2;
}

static int abi_t7(void) {
    struct R68 a0; fill_R68(&a0, 700u);
    struct R69 a1; fill_R69(&a1, 701u);
    struct R72 v0; fill_R72(&v0, 750u);
    struct R73 v1; fill_R73(&v1, 751u);
    int v2 = (int)((long long)(abi_mix(752u, 0u) % 4294967296u) - 2147483648);
    struct R74 v3; fill_R74(&v3, 753u);
    int bad = 0;
    abi_f7(a0, a1, v0, v1, v2, v3);
    abi_p7(a0, a1, v0, v1, v2, v3);
    if (bad) abi_fail(7, 99);
    return 2;
}

int main(void) {
    int calls = 0;
    if (abi_layout(0) != sizeof(struct R0) * 1000 + _Alignof(struct R0)) abi_fail(-1, 0);
    if (abi_layout(1) != sizeof(struct R1) * 1000 + _Alignof(struct R1)) abi_fail(-1, 1);
    if (abi_layout(2) != sizeof(struct R2) * 1000 + _Alignof(struct R2)) abi_fail(-1, 2);
    if (abi_layout(3) != sizeof(struct R3) * 1000 + _Alignof(struct R3)) abi_fail(-1, 3);
    if (abi_layout(4) != sizeof(union R4) * 1000 + _Alignof(union R4)) abi_fail(-1, 4);
    if (abi_layout(5) != sizeof(struct R5) * 1000 + _Alignof(struct R5)) abi_fail(-1, 5);
    if (abi_layout(6) != sizeof(struct R6) * 1000 + _Alignof(struct R6)) abi_fail(-1, 6);
    if (abi_layout(7) != sizeof(struct R7) * 1000 + _Alignof(struct R7)) abi_fail(-1, 7);
    if (abi_layout(8) != sizeof(struct R8) * 1000 + _Alignof(struct R8)) abi_fail(-1, 8);
    if (abi_layout(9) != sizeof(union R9) * 1000 + _Alignof(union R9)) abi_fail(-1, 9);
    if (abi_layout(10) != sizeof(struct R10) * 1000 + _Alignof(struct R10)) abi_fail(-1, 10);
    if (abi_layout(11) != sizeof(struct R11) * 1000 + _Alignof(struct R11)) abi_fail(-1, 11);
    if (abi_layout(12) != sizeof(struct R12) * 1000 + _Alignof(struct R12)) abi_fail(-1, 12);
    if (abi_layout(13) != sizeof(struct R13) * 1000 + _Alignof(struct R13)) abi_fail(-1, 13);
    if (abi_layout(14) != sizeof(struct R14) * 1000 + _Alignof(struct R14)) abi_fail(-1, 14);
    if (abi_layout(15) != sizeof(struct R15) * 1000 + _Alignof(struct R15)) abi_fail(-1, 15);
    if (abi_layout(16) != sizeof(struct R16) * 1000 + _Alignof(struct R16)) abi_fail(-1, 16);
    if (abi_layout(17) != sizeof(struct R17) * 1000 + _Alignof(struct R17)) abi_fail(-1, 17);
    if (abi_layout(18) != sizeof(union R18) * 1000 + _Alignof(union R18)) abi_fail(-1, 18);
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
    if (abi_layout(39) != sizeof(struct R39) * 1000 + _Alignof(struct R39)) abi_fail(-1, 39);
    if (abi_layout(40) != sizeof(struct R40) * 1000 + _Alignof(struct R40)) abi_fail(-1, 40);
    if (abi_layout(41) != sizeof(struct R41) * 1000 + _Alignof(struct R41)) abi_fail(-1, 41);
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
    if (abi_layout(53) != sizeof(struct R53) * 1000 + _Alignof(struct R53)) abi_fail(-1, 53);
    if (abi_layout(54) != sizeof(struct R54) * 1000 + _Alignof(struct R54)) abi_fail(-1, 54);
    if (abi_layout(55) != sizeof(struct R55) * 1000 + _Alignof(struct R55)) abi_fail(-1, 55);
    if (abi_layout(56) != sizeof(struct R56) * 1000 + _Alignof(struct R56)) abi_fail(-1, 56);
    if (abi_layout(57) != sizeof(struct R57) * 1000 + _Alignof(struct R57)) abi_fail(-1, 57);
    if (abi_layout(58) != sizeof(struct R58) * 1000 + _Alignof(struct R58)) abi_fail(-1, 58);
    if (abi_layout(59) != sizeof(struct R59) * 1000 + _Alignof(struct R59)) abi_fail(-1, 59);
    if (abi_layout(60) != sizeof(union R60) * 1000 + _Alignof(union R60)) abi_fail(-1, 60);
    if (abi_layout(61) != sizeof(struct R61) * 1000 + _Alignof(struct R61)) abi_fail(-1, 61);
    if (abi_layout(62) != sizeof(struct R62) * 1000 + _Alignof(struct R62)) abi_fail(-1, 62);
    if (abi_layout(63) != sizeof(struct R63) * 1000 + _Alignof(struct R63)) abi_fail(-1, 63);
    if (abi_layout(64) != sizeof(struct R64) * 1000 + _Alignof(struct R64)) abi_fail(-1, 64);
    if (abi_layout(65) != sizeof(struct R65) * 1000 + _Alignof(struct R65)) abi_fail(-1, 65);
    if (abi_layout(66) != sizeof(struct R66) * 1000 + _Alignof(struct R66)) abi_fail(-1, 66);
    if (abi_layout(67) != sizeof(struct R67) * 1000 + _Alignof(struct R67)) abi_fail(-1, 67);
    if (abi_layout(68) != sizeof(struct R68) * 1000 + _Alignof(struct R68)) abi_fail(-1, 68);
    if (abi_layout(69) != sizeof(struct R69) * 1000 + _Alignof(struct R69)) abi_fail(-1, 69);
    if (abi_layout(70) != sizeof(struct R70) * 1000 + _Alignof(struct R70)) abi_fail(-1, 70);
    if (abi_layout(71) != sizeof(struct R71) * 1000 + _Alignof(struct R71)) abi_fail(-1, 71);
    if (abi_layout(72) != sizeof(struct R72) * 1000 + _Alignof(struct R72)) abi_fail(-1, 72);
    if (abi_layout(73) != sizeof(struct R73) * 1000 + _Alignof(struct R73)) abi_fail(-1, 73);
    if (abi_layout(74) != sizeof(struct R74) * 1000 + _Alignof(struct R74)) abi_fail(-1, 74);
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
