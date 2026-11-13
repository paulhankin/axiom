/* abi_gen_106.c - written by gen_c_abi.py 105 8: calls of the
   functions of abi_gen_106_callee.c, directly, through pointers and with
   variadic arguments; prints the failures and the calls made. */
/* flags: --c-std=gnu23 */
/* units: abi/abi_gen_106_callee.c */
#include "host.h"
#include "abi/abi_gen_106.h"

int abi_result(void);
int abi_first(void);
unsigned long long abi_layout(int k);

static struct R0 (*volatile abi_p0)(struct R3 a0, ...) = abi_f0;
static unsigned long long (*volatile abi_p1)(struct R15 a0, struct R16 a1, union R17 a2, ...) = abi_f1;
static unsigned long long (*volatile abi_p2)(abi_v8uc a0, unsigned char a1, abi_v2ll a2) = abi_f2;
static void (*volatile abi_p3)(struct R31 a0, ...) = abi_f3;
static struct R42 (*volatile abi_p4)(int a0, abi_v2d a1, _Float16 a2, int a3, float a4, _Float128 a5, abi_v2d a6, struct R43 a7, _Float16 a8, abi_v4c a9, struct R44 a10, struct R47 a11, struct R48 a12, struct R49 a13, float a14) = abi_f4;
static struct R50 (*volatile abi_p5)(char * a0, struct R59 a1, struct R60 a2, struct R61 a3, ...) = abi_f5;
static struct R74 (*volatile abi_p6)(struct R76 a0, abi_v4f a1, struct R77 a2, struct R79 a3) = abi_f6;
static void (*volatile abi_p7)(double a0, int a1, struct R80 a2) = abi_f7;

static int abi_t0(void) {
    struct R3 a0; fill_R3(&a0, 0u);
    int v0 = (int)((long long)(abi_mix(50u, 0u) % 4294967296u) - 2147483648);
    struct R5 v1; fill_R5(&v1, 51u);
    struct R10 v2; fill_R10(&v2, 52u);
    struct R12 v3; fill_R12(&v3, 53u);
    struct R13 v4; fill_R13(&v4, 54u);
    struct R14 v5; fill_R14(&v5, 55u);
    int bad = 0;
    {
        struct R0 r = abi_f0(a0, v0, v1, v2, v3, v4, v5);
        if (!(check_R0(&r, 99u))) bad = 1;
    }
    {
        struct R0 r = abi_p0(a0, v0, v1, v2, v3, v4, v5);
        if (!(check_R0(&r, 99u))) bad = 1;
    }
    if (bad) abi_fail(0, 99);
    return 2;
}

static int abi_t1(void) {
    struct R15 a0; fill_R15(&a0, 100u);
    struct R16 a1; fill_R16(&a1, 101u);
    union R17 a2; fill_R17(&a2, 102u);
    struct R18 v0; fill_R18(&v0, 150u);
    long long v1 = (long long)abi_mix(151u, 0u);
    struct R24 v2; fill_R24(&v2, 152u);
    _Float128 v3 = ((_Float128)(long long)(abi_mix(153u, 0u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(153u, 0u) % 7u) * 0x1p-100f128);
    struct R25 v4; fill_R25(&v4, 154u);
    struct R27 v5; fill_R27(&v5, 155u);
    struct R28 v6; fill_R28(&v6, 156u);
    int bad = 0;
    {
        unsigned long long r = abi_f1(a0, a1, a2, v0, v1, v2, v3, v4, v5, v6);
        if (!(r == (unsigned long long)abi_mix(199u, 0u))) bad = 1;
    }
    {
        unsigned long long r = abi_p1(a0, a1, a2, v0, v1, v2, v3, v4, v5, v6);
        if (!(r == (unsigned long long)abi_mix(199u, 0u))) bad = 1;
    }
    if (bad) abi_fail(1, 99);
    return 2;
}

static int abi_t2(void) {
    abi_v8uc a0; fill_abi_v8uc(&a0, 200u);
    unsigned char a1 = (unsigned char)(abi_mix(201u, 0u) % 256u);
    abi_v2ll a2; fill_abi_v2ll(&a2, 202u);
    int bad = 0;
    {
        unsigned long long r = abi_f2(a0, a1, a2);
        if (!(r == (unsigned long long)abi_mix(299u, 0u))) bad = 1;
    }
    {
        unsigned long long r = abi_p2(a0, a1, a2);
        if (!(r == (unsigned long long)abi_mix(299u, 0u))) bad = 1;
    }
    if (bad) abi_fail(2, 99);
    return 2;
}

static int abi_t3(void) {
    struct R31 a0; fill_R31(&a0, 300u);
    union R32 v0; fill_R32(&v0, 350u);
    struct R34 v1; fill_R34(&v1, 351u);
    struct R36 v2; fill_R36(&v2, 352u);
    struct R37 v3; fill_R37(&v3, 353u);
    struct R39 v4; fill_R39(&v4, 354u);
    union R40 v5; fill_R40(&v5, 355u);
    int bad = 0;
    abi_f3(a0, v0, v1, v2, v3, v4, v5);
    abi_p3(a0, v0, v1, v2, v3, v4, v5);
    if (bad) abi_fail(3, 99);
    return 2;
}

static int abi_t4(void) {
    int a0 = (int)((long long)(abi_mix(400u, 0u) % 4294967296u) - 2147483648);
    abi_v2d a1; fill_abi_v2d(&a1, 401u);
    _Float16 a2 = (_Float16)((double)(long long)(abi_mix(402u, 0u) % 4001u) / 8.0 - 250.0);
    int a3 = (int)((long long)(abi_mix(403u, 0u) % 4294967296u) - 2147483648);
    float a4 = (float)((double)(long long)(abi_mix(404u, 0u) % 4001u) / 8.0 - 250.0);
    _Float128 a5 = ((_Float128)(long long)(abi_mix(405u, 0u) % 4001u) / 8 - 250 + (_Float128)(abi_mix(405u, 0u) % 7u) * 0x1p-100f128);
    abi_v2d a6; fill_abi_v2d(&a6, 406u);
    struct R43 a7; fill_R43(&a7, 407u);
    _Float16 a8 = (_Float16)((double)(long long)(abi_mix(408u, 0u) % 4001u) / 8.0 - 250.0);
    abi_v4c a9; fill_abi_v4c(&a9, 409u);
    struct R44 a10; fill_R44(&a10, 410u);
    struct R47 a11; fill_R47(&a11, 411u);
    struct R48 a12; fill_R48(&a12, 412u);
    struct R49 a13; fill_R49(&a13, 413u);
    float a14 = (float)((double)(long long)(abi_mix(414u, 0u) % 4001u) / 8.0 - 250.0);
    int bad = 0;
    {
        struct R42 r = abi_f4(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
        if (!(check_R42(&r, 499u))) bad = 1;
    }
    {
        struct R42 r = abi_p4(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
        if (!(check_R42(&r, 499u))) bad = 1;
    }
    if (bad) abi_fail(4, 99);
    return 2;
}

static int abi_t5(void) {
    char * a0 = (abi_anchor + (abi_mix(500u, 0u) % 8u));
    struct R59 a1; fill_R59(&a1, 501u);
    struct R60 a2; fill_R60(&a2, 502u);
    struct R61 a3; fill_R61(&a3, 503u);
    abi_v2f v0; fill_abi_v2f(&v0, 550u);
    union R62 v1; fill_R62(&v1, 551u);
    struct R66 v2; fill_R66(&v2, 552u);
    struct R71 v3; fill_R71(&v3, 553u);
    unsigned long long v4 = (unsigned long long)abi_mix(554u, 0u);
    struct R73 v5; fill_R73(&v5, 555u);
    int bad = 0;
    {
        struct R50 r = abi_f5(a0, a1, a2, a3, v0, v1, v2, v3, v4, v5);
        if (!(check_R50(&r, 599u))) bad = 1;
    }
    {
        struct R50 r = abi_p5(a0, a1, a2, a3, v0, v1, v2, v3, v4, v5);
        if (!(check_R50(&r, 599u))) bad = 1;
    }
    if (bad) abi_fail(5, 99);
    return 2;
}

static int abi_t6(void) {
    struct R76 a0; fill_R76(&a0, 600u);
    abi_v4f a1; fill_abi_v4f(&a1, 601u);
    struct R77 a2; fill_R77(&a2, 602u);
    struct R79 a3; fill_R79(&a3, 603u);
    int bad = 0;
    {
        struct R74 r = abi_f6(a0, a1, a2, a3);
        if (!(check_R74(&r, 699u))) bad = 1;
    }
    {
        struct R74 r = abi_p6(a0, a1, a2, a3);
        if (!(check_R74(&r, 699u))) bad = 1;
    }
    if (bad) abi_fail(6, 99);
    return 2;
}

static int abi_t7(void) {
    double a0 = (double)((double)(long long)(abi_mix(700u, 0u) % 4001u) / 8.0 - 250.0);
    int a1 = (int)((long long)(abi_mix(701u, 0u) % 4294967296u) - 2147483648);
    struct R80 a2; fill_R80(&a2, 702u);
    int bad = 0;
    abi_f7(a0, a1, a2);
    abi_p7(a0, a1, a2);
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
    if (abi_layout(6) != sizeof(union R6) * 1000 + _Alignof(union R6)) abi_fail(-1, 6);
    if (abi_layout(7) != sizeof(struct R7) * 1000 + _Alignof(struct R7)) abi_fail(-1, 7);
    if (abi_layout(8) != sizeof(struct R8) * 1000 + _Alignof(struct R8)) abi_fail(-1, 8);
    if (abi_layout(9) != sizeof(struct R9) * 1000 + _Alignof(struct R9)) abi_fail(-1, 9);
    if (abi_layout(10) != sizeof(struct R10) * 1000 + _Alignof(struct R10)) abi_fail(-1, 10);
    if (abi_layout(11) != sizeof(struct R11) * 1000 + _Alignof(struct R11)) abi_fail(-1, 11);
    if (abi_layout(12) != sizeof(struct R12) * 1000 + _Alignof(struct R12)) abi_fail(-1, 12);
    if (abi_layout(13) != sizeof(struct R13) * 1000 + _Alignof(struct R13)) abi_fail(-1, 13);
    if (abi_layout(14) != sizeof(struct R14) * 1000 + _Alignof(struct R14)) abi_fail(-1, 14);
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
    if (abi_layout(30) != sizeof(union R30) * 1000 + _Alignof(union R30)) abi_fail(-1, 30);
    if (abi_layout(31) != sizeof(struct R31) * 1000 + _Alignof(struct R31)) abi_fail(-1, 31);
    if (abi_layout(32) != sizeof(union R32) * 1000 + _Alignof(union R32)) abi_fail(-1, 32);
    if (abi_layout(33) != sizeof(struct R33) * 1000 + _Alignof(struct R33)) abi_fail(-1, 33);
    if (abi_layout(34) != sizeof(struct R34) * 1000 + _Alignof(struct R34)) abi_fail(-1, 34);
    if (abi_layout(35) != sizeof(struct R35) * 1000 + _Alignof(struct R35)) abi_fail(-1, 35);
    if (abi_layout(36) != sizeof(struct R36) * 1000 + _Alignof(struct R36)) abi_fail(-1, 36);
    if (abi_layout(37) != sizeof(struct R37) * 1000 + _Alignof(struct R37)) abi_fail(-1, 37);
    if (abi_layout(38) != sizeof(struct R38) * 1000 + _Alignof(struct R38)) abi_fail(-1, 38);
    if (abi_layout(39) != sizeof(struct R39) * 1000 + _Alignof(struct R39)) abi_fail(-1, 39);
    if (abi_layout(40) != sizeof(union R40) * 1000 + _Alignof(union R40)) abi_fail(-1, 40);
    if (abi_layout(41) != sizeof(struct R41) * 1000 + _Alignof(struct R41)) abi_fail(-1, 41);
    if (abi_layout(42) != sizeof(struct R42) * 1000 + _Alignof(struct R42)) abi_fail(-1, 42);
    if (abi_layout(43) != sizeof(struct R43) * 1000 + _Alignof(struct R43)) abi_fail(-1, 43);
    if (abi_layout(44) != sizeof(struct R44) * 1000 + _Alignof(struct R44)) abi_fail(-1, 44);
    if (abi_layout(45) != sizeof(union R45) * 1000 + _Alignof(union R45)) abi_fail(-1, 45);
    if (abi_layout(46) != sizeof(struct R46) * 1000 + _Alignof(struct R46)) abi_fail(-1, 46);
    if (abi_layout(47) != sizeof(struct R47) * 1000 + _Alignof(struct R47)) abi_fail(-1, 47);
    if (abi_layout(48) != sizeof(struct R48) * 1000 + _Alignof(struct R48)) abi_fail(-1, 48);
    if (abi_layout(49) != sizeof(struct R49) * 1000 + _Alignof(struct R49)) abi_fail(-1, 49);
    if (abi_layout(50) != sizeof(struct R50) * 1000 + _Alignof(struct R50)) abi_fail(-1, 50);
    if (abi_layout(51) != sizeof(struct R51) * 1000 + _Alignof(struct R51)) abi_fail(-1, 51);
    if (abi_layout(52) != sizeof(union R52) * 1000 + _Alignof(union R52)) abi_fail(-1, 52);
    if (abi_layout(53) != sizeof(struct R53) * 1000 + _Alignof(struct R53)) abi_fail(-1, 53);
    if (abi_layout(54) != sizeof(struct R54) * 1000 + _Alignof(struct R54)) abi_fail(-1, 54);
    if (abi_layout(55) != sizeof(struct R55) * 1000 + _Alignof(struct R55)) abi_fail(-1, 55);
    if (abi_layout(56) != sizeof(struct R56) * 1000 + _Alignof(struct R56)) abi_fail(-1, 56);
    if (abi_layout(57) != sizeof(struct R57) * 1000 + _Alignof(struct R57)) abi_fail(-1, 57);
    if (abi_layout(58) != sizeof(union R58) * 1000 + _Alignof(union R58)) abi_fail(-1, 58);
    if (abi_layout(59) != sizeof(struct R59) * 1000 + _Alignof(struct R59)) abi_fail(-1, 59);
    if (abi_layout(60) != sizeof(struct R60) * 1000 + _Alignof(struct R60)) abi_fail(-1, 60);
    if (abi_layout(61) != sizeof(struct R61) * 1000 + _Alignof(struct R61)) abi_fail(-1, 61);
    if (abi_layout(62) != sizeof(union R62) * 1000 + _Alignof(union R62)) abi_fail(-1, 62);
    if (abi_layout(63) != sizeof(struct R63) * 1000 + _Alignof(struct R63)) abi_fail(-1, 63);
    if (abi_layout(64) != sizeof(union R64) * 1000 + _Alignof(union R64)) abi_fail(-1, 64);
    if (abi_layout(65) != sizeof(union R65) * 1000 + _Alignof(union R65)) abi_fail(-1, 65);
    if (abi_layout(66) != sizeof(struct R66) * 1000 + _Alignof(struct R66)) abi_fail(-1, 66);
    if (abi_layout(67) != sizeof(struct R67) * 1000 + _Alignof(struct R67)) abi_fail(-1, 67);
    if (abi_layout(68) != sizeof(struct R68) * 1000 + _Alignof(struct R68)) abi_fail(-1, 68);
    if (abi_layout(69) != sizeof(struct R69) * 1000 + _Alignof(struct R69)) abi_fail(-1, 69);
    if (abi_layout(70) != sizeof(struct R70) * 1000 + _Alignof(struct R70)) abi_fail(-1, 70);
    if (abi_layout(71) != sizeof(struct R71) * 1000 + _Alignof(struct R71)) abi_fail(-1, 71);
    if (abi_layout(72) != sizeof(struct R72) * 1000 + _Alignof(struct R72)) abi_fail(-1, 72);
    if (abi_layout(73) != sizeof(struct R73) * 1000 + _Alignof(struct R73)) abi_fail(-1, 73);
    if (abi_layout(74) != sizeof(struct R74) * 1000 + _Alignof(struct R74)) abi_fail(-1, 74);
    if (abi_layout(75) != sizeof(struct R75) * 1000 + _Alignof(struct R75)) abi_fail(-1, 75);
    if (abi_layout(76) != sizeof(struct R76) * 1000 + _Alignof(struct R76)) abi_fail(-1, 76);
    if (abi_layout(77) != sizeof(struct R77) * 1000 + _Alignof(struct R77)) abi_fail(-1, 77);
    if (abi_layout(78) != sizeof(struct R78) * 1000 + _Alignof(struct R78)) abi_fail(-1, 78);
    if (abi_layout(79) != sizeof(struct R79) * 1000 + _Alignof(struct R79)) abi_fail(-1, 79);
    if (abi_layout(80) != sizeof(struct R80) * 1000 + _Alignof(struct R80)) abi_fail(-1, 80);
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
